#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <fstream>

struct vec3 {
    float x;
    float y;
    float z;
};

class FileStream {
public:
    FileStream(const std::string& filename, const char* mode);
    ~FileStream();

    operator bool() const;
    bool eof() const;
    void skip(size_t bytes);
    void seek(size_t position, int origin);

    template<typename T>
    T read();

    std::string readString();

private:
    std::FILE* mFile;
};

class CatalogItem {
public:
    std::string assetNameWType;
    int64_t compileTime = 0;       // Inicializado
    uint32_t dataCrc = 0;          // Inicializado
    uint32_t typeCrc = 0;          // Inicializado
    std::string sourceFileNameWType;
    int32_t version = 0;           // Inicializado
    std::vector<std::string> tags;

    void read(FileStream& stream);
    void write(FileStream& stream) const;
};

class Catalog {
public:
    bool read(const std::string& filename);
    void write(const std::string& filename) const;

    const std::vector<CatalogItem>& getItems() const { return mItems; }

private:
    std::vector<CatalogItem> mItems;
};

class Type {
public:
    struct Field {
        std::string name;
        std::shared_ptr<Type> type;
        size_t offset;
        bool useKeyData;
    };

    Type(std::string name, uint32_t size);
    const std::string& getName() const { return mName; }
    uint32_t getSize() const { return mSize; }
    const std::vector<Field>& getFields() const { return mFields; }

    void addField(const std::string& name, std::shared_ptr<Type> type, size_t offset = 0, bool useKeyData = false);

private:
    std::string mName;
    uint32_t mSize;
    std::vector<Field> mFields;
};

class TypeDatabase {
public:
    static TypeDatabase& instance();

    std::shared_ptr<Type> add(std::string name, uint32_t size);
    std::shared_ptr<Type> addStruct(std::string name);

    std::shared_ptr<Type> getType(const std::string& name);
    std::shared_ptr<Type> getTypeByHash(uint32_t hash);

    void initialize();

private:
    TypeDatabase();

    std::unordered_map<std::string, std::shared_ptr<Type>> mTypes;
    std::unordered_map<uint32_t, std::shared_ptr<Type>> mTypesByHash;

    static TypeDatabase sInstance;
};

uint32_t HashFunction(const char* str, uint32_t seed, uint32_t len);