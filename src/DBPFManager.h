#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <filesystem>

struct ResourceKey {
    uint32_t typeID;
    uint32_t groupID;
    uint32_t instanceID;

    ResourceKey();
    ResourceKey(uint32_t t, uint32_t g, uint32_t i);

    bool operator==(const ResourceKey& other) const;
};

class DbpfArchive {
public:
    explicit DbpfArchive(const std::filesystem::path& packagePath);
    ~DbpfArchive();

    void loadRegistries(const std::filesystem::path& registryDir);
    std::vector<std::string> listAssets() const;
    bool getAsset(const std::string& virtualName, std::vector<uint8_t>& outData);
    bool getAssetByKey(const ResourceKey& key, std::vector<uint8_t>& outData);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};