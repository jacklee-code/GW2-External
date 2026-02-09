#pragma once

#include <string>
#include <vector>
#include <array>

struct Teleport {
    std::string name;
    std::array<float, 3> coordinates; // X, Y, Z
    int mapId;

    Teleport(const std::string& n = "Teleport", float x = 0.0f, float y = 0.0f, float z = 0.0f, int map = -1)
        : name(n), coordinates{ x, y, z }, mapId(map) {}
};

struct TeleportGroup {
    std::string name;
    std::vector<Teleport> teleports;

    TeleportGroup(const std::string& n = "Group") : name(n) {}
};

class TeleportManager {
public:
    TeleportManager();
    ~TeleportManager();

    // Group management
    bool addGroup(const std::string& groupName);
    bool deleteGroup(size_t groupIndex);
    bool renameGroup(size_t groupIndex, const std::string& newName);
    size_t getGroupCount() const;
    const TeleportGroup* getGroup(size_t index) const;
    TeleportGroup* getGroup(size_t index);

    // Teleport management
    bool addTeleport(size_t groupIndex, const Teleport& teleport);
    bool deleteTeleport(size_t groupIndex, size_t teleportIndex);
    bool updateTeleport(size_t groupIndex, size_t teleportIndex, const Teleport& teleport);
    std::string generateUniqueTeleportName(size_t groupIndex) const;

    // File operations
    bool loadFromFile(const std::string& filepath);
    bool saveToFile(const std::string& filepath) const;
    bool importFromJson(const std::string& jsonContent);
    bool importFromJsonScaled(const std::string& jsonContent, float scaleFactor);
    std::string exportToJson(size_t groupIndex) const;

private:
    std::vector<TeleportGroup> m_groups;
    std::string m_configPath;

    void ensureConfigDirectory();
    bool validateImportedJson(const std::string& jsonContent) const;
};
