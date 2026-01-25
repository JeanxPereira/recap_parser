#include "DBPFManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <sstream>
#include <cstring>
#include <cctype>

ResourceKey::ResourceKey() : typeID(0), groupID(0), instanceID(0) {}
ResourceKey::ResourceKey(uint32_t t, uint32_t g, uint32_t i) : typeID(t), groupID(g), instanceID(i) {}
bool ResourceKey::operator==(const ResourceKey& other) const {
    return typeID == other.typeID && groupID == other.groupID && instanceID == other.instanceID;
}

namespace {

    static inline void trim(std::string& s) {
        auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
        auto it1 = std::find_if_not(s.begin(), s.end(), [&](char c) { return is_space(static_cast<unsigned char>(c)); });
        auto it2 = std::find_if_not(s.rbegin(), s.rend(), [&](char c) { return is_space(static_cast<unsigned char>(c)); }).base();
        if (it1 >= it2) { s.clear(); return; }
        s.assign(it1, it2);
    }

    static int32_t fnv_hash_local(const std::string& s) {
        if (s.empty()) return 0;
        std::string lower;
        lower.reserve(s.size());
        for (char c : s) lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        uint32_t rez = 0x811C9DC5u;
        for (char c : lower) {
            rez *= 0x1000193u;
            rez ^= static_cast<uint8_t>(c);
        }
        return static_cast<int32_t>(rez);
    }

    static int32_t int32_local(const std::string& str) {
        std::string s = str;
        trim(s);
        if (s.empty()) return 0;
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            try { return static_cast<int32_t>(std::stoul(s, nullptr, 16)); }
            catch (...) { return 0; }
        }
        if (!s.empty() && s[0] == '#') {
            try { return static_cast<int32_t>(std::stoul(s.substr(1), nullptr, 16)); }
            catch (...) { return 0; }
        }
        if (!s.empty() && s[0] == '$') {
            return fnv_hash_local(s.substr(1));
        }
        try { return std::stoi(s); }
        catch (...) { return 0; }
    }

    class RefPackCompression {
    public:
        static void DecompressFast(const std::uint8_t* input, std::size_t inputSize, std::uint8_t* output, std::size_t outputSize) {
            if (inputSize < 5) return;
            std::size_t pin = 0;
            std::uint8_t cType = input[pin++];
            pin++;
            if (cType != 0x10 && cType != 0x50) return;

            int decompSize = ((input[pin] & 0xFF) << 16) | ((input[pin + 1] & 0xFF) << 8) | (input[pin + 2] & 0xFF);
            pin += 3;

            if (outputSize < static_cast<std::size_t>(decompSize)) return;

            size_t size = 0;
            while (size < static_cast<size_t>(decompSize)) {
                if (pin >= inputSize) break;

                int ctrl = input[pin++] & 0xFF;
                int numPlain = 0;
                int numCopy = 0;
                int copyOff = 0;

                if (ctrl >= 252) {
                    numPlain = ctrl & 0x03;
                }
                else if (ctrl >= 224) {
                    numPlain = ((ctrl & 0x1F) << 2) + 4;
                }
                else if (ctrl >= 192) {
                    if (pin + 3 > inputSize) break;
                    std::uint8_t b1 = input[pin++];
                    std::uint8_t b2 = input[pin++];
                    std::uint8_t b3 = input[pin++];
                    numPlain = ctrl & 0x03;
                    numCopy = ((ctrl & 0x0C) << 6) + (b3 & 0xFF) + 5;
                    copyOff = ((ctrl & 0x10) << 12) + ((b1 & 0xFF) << 8) + (b2 & 0xFF) + 1;
                }
                else if (ctrl >= 128) {
                    if (pin + 2 > inputSize) break;
                    std::uint8_t b2 = input[pin++];
                    std::uint8_t b3 = input[pin++];
                    numPlain = ((b2 & 0xC0) >> 6) & 0x03;
                    numCopy = (ctrl & 0x3F) + 4;
                    copyOff = ((b2 & 0x3F) << 8) + (b3 & 0xFF) + 1;
                }
                else {
                    if (pin + 1 > inputSize) break;
                    std::uint8_t b1 = input[pin++];
                    numPlain = ctrl & 0x03;
                    numCopy = ((ctrl & 0x1C) >> 2) + 3;
                    copyOff = ((ctrl & 0x60) << 3) + (b1 & 0xFF) + 1;
                }

                if (numPlain > 0) {
                    if (pin + numPlain > inputSize) break;
                    if (size + numPlain > outputSize) break;
                    std::memcpy(output + size, input + pin, numPlain);
                    pin += numPlain;
                    size += numPlain;
                }

                if (numCopy > 0) {
                    if (copyOff <= 0 || copyOff > (int)size) break;
                    if (size + numCopy > outputSize) break;

                    if (numCopy > copyOff) {
                        for (int c = 0; c < numCopy; ++c) {
                            output[size] = output[size - copyOff];
                            size++;
                        }
                    }
                    else {
                        std::memcpy(output + size, output + size - copyOff, numCopy);
                        size += numCopy;
                    }
                }
            }
        }
    };

    class FileReadStream {
        std::ifstream file;
        std::int64_t baseOffset;
    public:
        explicit FileReadStream(const std::filesystem::path& path) : file(path, std::ios::binary), baseOffset(0) {
            if (!file) throw std::runtime_error("");
        }
        void Seek(std::int64_t off) { file.seekg(off + baseOffset, std::ios::beg); }
        void Skip(std::int64_t n) { file.seekg(n, std::ios::cur); }
        std::int64_t GetFilePointer() { return static_cast<std::int64_t>(file.tellg()) - baseOffset; }
        void ReadBytes(std::uint8_t* dst, std::size_t len) { file.read((char*)dst, len); }
        std::uint8_t ReadByte() { char c; file.read(&c, 1); return (std::uint8_t)c; }
        bool ReadBoolean() { return ReadByte() != 0; }

        std::int16_t ReadLEShort() {
            std::uint8_t b[2]; ReadBytes(b, 2);
            return (std::int16_t)((b[0]) | (b[1] << 8));
        }
        std::int32_t ReadLEInt() {
            std::uint8_t b[4]; ReadBytes(b, 4);
            return (std::int32_t)((b[0]) | (b[1] << 8) | (b[2] << 16) | (b[3] << 24));
        }
        std::uint32_t ReadLEUInt() { return (std::uint32_t)ReadLEInt(); }
        std::int64_t ReadLELong() {
            std::uint8_t b[8]; ReadBytes(b, 8);
            uint64_t v = 0;
            v |= (uint64_t)b[0]; v |= (uint64_t)b[1] << 8; v |= (uint64_t)b[2] << 16; v |= (uint64_t)b[3] << 24;
            v |= (uint64_t)b[4] << 32; v |= (uint64_t)b[5] << 40; v |= (uint64_t)b[6] << 48; v |= (uint64_t)b[7] << 56;
            return (std::int64_t)v;
        }
    };

    class NameRegistry {
    public:
        std::unordered_map<std::string, int32_t> hashes;
        std::unordered_map<int32_t, std::string> names;

        void Clear() { hashes.clear(); names.clear(); }

        void Add(const std::string& name, int32_t hash) {
            std::string lower = name;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
            hashes[lower] = hash;
            names[hash] = name;
        }

        void Read(std::istream& in) {
            std::string line;
            while (std::getline(in, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                size_t cPos = line.find("//");
                if (cPos != std::string::npos) line = line.substr(0, cPos);
                if (line.empty()) continue;

                for (char& c : line) if (c == '\t') c = ' ';
                std::stringstream ss(line);
                std::string name, hashStr;
                if (ss >> name >> hashStr) {
                    Add(name, int32_local(hashStr));
                }
            }
        }

        void Load(const std::filesystem::path& path) {
            std::ifstream in(path);
            if (in) Read(in);
        }

        const std::string* GetName(int32_t hash) const {
            auto it = names.find(hash);
            return it != names.end() ? &it->second : nullptr;
        }

        bool GetHash(const std::string& name, int32_t& out) const {
            std::string lower = name;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
            auto it = hashes.find(lower);
            if (it != hashes.end()) { out = it->second; return true; }
            return false;
        }
    };

    struct DBPFItem {
        ResourceKey name;
        uint64_t chunkOffset;
        int memSize;
        int compressedSize;
        bool isCompressed;
        void Read(FileReadStream& s, bool isDBBF, bool rt, bool rg, int tID, int gID) {
            name.typeID = rt ? s.ReadLEInt() : tID;
            name.groupID = rg ? s.ReadLEInt() : gID;
            name.instanceID = s.ReadLEInt();
            chunkOffset = isDBBF ? (uint64_t)s.ReadLELong() : (uint64_t)s.ReadLEUInt();
            compressedSize = s.ReadLEInt() & 0x7fffffff;
            memSize = s.ReadLEInt();
            int16_t c = s.ReadLEShort();
            isCompressed = (c == -1 || c != 0);
            s.ReadBoolean();
            s.Skip(1);
        }
    };

}

struct DbpfArchive::Impl {
    std::filesystem::path path;
    std::vector<DBPFItem> items;
    std::vector<ResourceKey> keys;
    NameRegistry fileReg, typeReg, projectReg;
    std::mutex mutex;

    Impl(const std::filesystem::path& p) : path(p) {
        LoadIndex();
        try { LoadInternalNames(); }
        catch (...) {}
    }

    void LoadIndex() {
        FileReadStream s(path);
        int magic = s.ReadLEInt();  // offset 0x00
        bool isDBBF = (magic == 0x46424244); // "DBBF"
        bool isDBPF = (magic == 0x46504244); // "DBPF"
        
        std::cerr << "[LoadIndex] magic=0x" << std::hex << magic << std::dec 
                  << " isDBPF=" << isDBPF << " isDBBF=" << isDBBF << "\n";
        
        if (!isDBPF && !isDBBF) {
            std::cerr << "[LoadIndex] Unknown format, aborting\n";
            return;
        }

        int majorVersion = s.ReadLEInt();  // offset 0x04
        int minorVersion = s.ReadLEInt();  // offset 0x08
        std::cerr << "[LoadIndex] version=" << majorVersion << "." << minorVersion << "\n";

        int count = 0;
        int64_t indexOff = 0;
        
        if (isDBBF) {
            // DBBF format (64-bit offsets)
            s.Seek(0x24);
            count = s.ReadLEInt();        // 0x24
            s.Seek(0x30);
            indexOff = s.ReadLELong();    // 0x30 (64-bit)
        }
        else {
            // DBPF 3.0 format - matching server code
            s.Seek(0x24);
            count = s.ReadLEInt();        // 0x24: indexCount
            s.Seek(0x40);
            indexOff = s.ReadLEUInt();    // 0x40: indexOffset
        }
        
        std::cerr << "[LoadIndex] entry count=" << count << "\n";
        std::cerr << "[LoadIndex] indexOffset=" << indexOff << "\n";

        s.Seek(indexOff);
        int typeID = -1, groupID = -1;
        int flags = s.ReadLEInt();
        std::cerr << "[LoadIndex] flags=" << flags << "\n";
        
        if (flags & 1) typeID = s.ReadLEInt();
        if (flags & 2) groupID = s.ReadLEInt();
        if (flags & 4) s.ReadLEInt();

        items.reserve(count);
        keys.reserve(count);

        for (int i = 0; i < count; ++i) {
            DBPFItem it;
            it.Read(s, isDBBF, typeID == -1, groupID == -1, typeID, groupID);
            items.push_back(it);
            keys.push_back(it.name);
        }
        
        std::cerr << "[LoadIndex] loaded " << items.size() << " items\n";
    }

    void LoadInternalNames() {
        int32_t group = fnv_hash_local("sporemaster");
        int32_t name = fnv_hash_local("names");
        for (const auto& it : items) {
            if ((int32_t)it.name.groupID == group && (int32_t)it.name.instanceID == name) {
                std::vector<uint8_t> data;
                if (ReadData(it, data)) {
                    std::string txt((char*)data.data(), data.size());
                    std::stringstream ss(txt);
                    projectReg.Read(ss);
                }
                break;
            }
        }
    }

    bool ReadData(const DBPFItem& it, std::vector<uint8_t>& out) {
        std::lock_guard<std::mutex> lock(mutex);
        try {
            FileReadStream s(path);
            s.Seek(it.chunkOffset);
            std::vector<uint8_t> comp(it.compressedSize);
            s.ReadBytes(comp.data(), it.compressedSize);

            if (it.isCompressed) {
                out.resize(it.memSize);
                RefPackCompression::DecompressFast(comp.data(), comp.size(), out.data(), out.size());
            }
            else {
                out = std::move(comp);
            }
            return true;
        }
        catch (...) { return false; }
    }

    ResourceKey Resolve(const std::string& vName, bool debug = false) {
        std::string fname = vName;
        std::string ftype;
        size_t dot = fname.rfind('.');
        if (dot != std::string::npos) {
            ftype = fname.substr(dot + 1);
            fname = fname.substr(0, dot);
        }

        int32_t fHash = 0;
        bool foundInFileReg = fileReg.GetHash(fname, fHash);
        bool foundInProjectReg = !foundInFileReg && projectReg.GetHash(fname, fHash);
        
        if (!foundInFileReg && !foundInProjectReg) {
            fHash = int32_local(fname);
            if (fHash == 0 && fname != "0") fHash = fnv_hash_local(fname);
        }

        int32_t tHash = 0;
        bool foundTypeReg = false;
        if (!ftype.empty()) {
            foundTypeReg = typeReg.GetHash(ftype, tHash);
            if (!foundTypeReg) {
                tHash = int32_local(ftype);
                if (tHash == 0 && ftype != "0") tHash = fnv_hash_local(ftype);
            }
        }

        if (debug) {
            std::cerr << "[Resolve] Input: '" << vName << "'\n";
            std::cerr << "[Resolve] fname='" << fname << "' ftype='" << ftype << "'\n";
            std::cerr << "[Resolve] fHash=0x" << std::hex << (uint32_t)fHash 
                      << " (fileReg=" << foundInFileReg << ", projectReg=" << foundInProjectReg << ")\n";
            std::cerr << "[Resolve] tHash=0x" << std::hex << (uint32_t)tHash 
                      << " (typeReg=" << foundTypeReg << ")\n" << std::dec;
            std::cerr << "[Resolve] Searching " << keys.size() << " keys...\n";
        }

        if (tHash != 0) {
            for (const auto& k : keys) {
                if (k.instanceID == (uint32_t)fHash && k.typeID == (uint32_t)tHash) {
                    if (debug) std::cerr << "[Resolve] FOUND with type match!\n";
                    return k;
                }
            }
        }
        for (const auto& k : keys) {
            if (k.instanceID == (uint32_t)fHash) {
                if (debug) std::cerr << "[Resolve] FOUND by instanceID only\n";
                return k;
            }
        }
        
        if (debug) std::cerr << "[Resolve] NOT FOUND\n";
        return ResourceKey();
    }
};

DbpfArchive::DbpfArchive(const std::filesystem::path& p) : impl(std::make_unique<Impl>(p)) {}
DbpfArchive::~DbpfArchive() = default;

void DbpfArchive::loadRegistries(const std::filesystem::path& d) {
    impl->fileReg.Load(d / "reg_file.txt");
    impl->typeReg.Load(d / "reg_type.txt");
}

std::vector<std::string> DbpfArchive::listAssets() const {
    std::vector<std::string> ret;
    for (const auto& k : impl->keys) {
        const std::string* f = impl->fileReg.GetName(k.instanceID);
        if (!f) f = impl->projectReg.GetName(k.instanceID);
        const std::string* t = impl->typeReg.GetName(k.typeID);

        std::stringstream ss;
        if (f) ss << *f; else ss << "0x" << std::hex << k.instanceID;
        if (t) ss << "." << *t; else ss << ".0x" << std::hex << k.typeID;
        ret.push_back(ss.str());
    }
    return ret;
}

bool DbpfArchive::getAsset(const std::string& name, std::vector<uint8_t>& out) {
    ResourceKey k = impl->Resolve(name, false);
    if (k.instanceID == 0 && k.typeID == 0) {
        // Retry with debug to show what went wrong
        impl->Resolve(name, true);
        return false;
    }
    return getAssetByKey(k, out);
}

bool DbpfArchive::getAssetByKey(const ResourceKey& k, std::vector<uint8_t>& out) {
    for (const auto& it : impl->items) {
        if (it.name == k) return impl->ReadData(it, out);
    }
    return false;
}