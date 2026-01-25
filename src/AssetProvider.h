#pragma once

#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <unordered_set>

#include "DBPFManager.h"

class IAssetProvider {
public:
    virtual ~IAssetProvider() = default;

    virtual std::vector<std::string> listAssets() const = 0;

    virtual bool getAsset(const std::string& name, std::vector<uint8_t>& outData) = 0;
};

class DiskAssetProvider : public IAssetProvider {
private:
    std::filesystem::path rootPath;
    std::unordered_set<std::string> extensionFilter;

public:
    DiskAssetProvider(const std::filesystem::path& path, const std::string& filterStr = "")
        : rootPath(path) {
        if (!filterStr.empty()) {
            std::string tmp;
            for (char c : filterStr) {
                if (c == ',' || c == ';' || std::isspace(static_cast<unsigned char>(c))) {
                    if (!tmp.empty()) {
                        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
                        if (tmp[0] == '.') tmp.erase(0, 1);
                        extensionFilter.insert(tmp);
                        tmp.clear();
                    }
                }
                else {
                    tmp.push_back(c);
                }
            }
            if (!tmp.empty()) {
                std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
                if (tmp[0] == '.') tmp.erase(0, 1);
                extensionFilter.insert(tmp);
            }
        }
    }

    std::vector<std::string> listAssets() const override {
        std::vector<std::string> files;

        if (std::filesystem::is_regular_file(rootPath)) {
            files.push_back(rootPath.string());
            return files;
        }

        // Se for diretorio, busca recursiva
        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(rootPath)) {
            if (dir_entry.is_regular_file()) {
                if (!extensionFilter.empty()) {
                    std::string ext = dir_entry.path().extension().string();
                    if (!ext.empty() && ext[0] == '.') ext.erase(0, 1);
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (extensionFilter.count(ext) == 0) continue;
                }
                files.push_back(dir_entry.path().string());
            }
        }
        return files;
    }

    bool getAsset(const std::string& name, std::vector<uint8_t>& outData) override {
        std::ifstream file(name, std::ios::binary | std::ios::ate);
        if (!file) return false;

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size <= 0) return false;

        outData.resize(size);
        if (file.read(reinterpret_cast<char*>(outData.data()), size)) {
            return true;
        }
        return false;
    }
};

class DbpfAssetProvider : public IAssetProvider {
private:
    std::unique_ptr<DbpfArchive> archive;

public:
    DbpfAssetProvider(const std::filesystem::path& packagePath, const std::filesystem::path& registryDir) {
        archive = std::make_unique<DbpfArchive>(packagePath);
        if (!registryDir.empty() && std::filesystem::exists(registryDir)) {
            archive->loadRegistries(registryDir);
        }
    }

    std::vector<std::string> listAssets() const override {
        return archive->listAssets();
    }

    bool getAsset(const std::string& name, std::vector<uint8_t>& outData) override {
        return archive->getAsset(name, outData);
    }
};