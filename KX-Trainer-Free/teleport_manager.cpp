#include "teleport_manager.h"
#include "nlohmann/json.hpp"
#include "status_ui.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <sstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

TeleportManager::TeleportManager() {
    ensureConfigDirectory();
    
    // Set default config path
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    fs::path exeDir = fs::path(exePath).parent_path();
    m_configPath = (exeDir / "config" / "teleports.json").string();

    // Try to load existing config
    if (fs::exists(m_configPath)) {
        loadFromFile(m_configPath);
    } else {
        // Create default group if no config exists
        addGroup("Default Group");
    }
}

TeleportManager::~TeleportManager() {
    saveToFile(m_configPath);
}

void TeleportManager::ensureConfigDirectory() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    fs::path exeDir = fs::path(exePath).parent_path();
    fs::path configDir = exeDir / "config";

    if (!fs::exists(configDir)) {
        try {
            fs::create_directories(configDir);
        } catch (const std::exception& e) {
            StatusUI::AddMessage("ERROR: Failed to create config directory: " + std::string(e.what()));
        }
    }
}

bool TeleportManager::addGroup(const std::string& groupName) {
    // Check for duplicate names
    for (const auto& group : m_groups) {
        if (group.name == groupName) {
            StatusUI::AddMessage("ERROR: Group name already exists: " + groupName);
            return false;
        }
    }

    m_groups.emplace_back(groupName);
    saveToFile(m_configPath);
    return true;
}

bool TeleportManager::deleteGroup(size_t groupIndex) {
    if (groupIndex >= m_groups.size()) {
        return false;
    }

    m_groups.erase(m_groups.begin() + groupIndex);
    saveToFile(m_configPath);
    return true;
}

bool TeleportManager::renameGroup(size_t groupIndex, const std::string& newName) {
    if (groupIndex >= m_groups.size()) {
        return false;
    }

    // Check for duplicate names (excluding current group)
    for (size_t i = 0; i < m_groups.size(); ++i) {
        if (i != groupIndex && m_groups[i].name == newName) {
            StatusUI::AddMessage("ERROR: Group name already exists: " + newName);
            return false;
        }
    }

    m_groups[groupIndex].name = newName;
    saveToFile(m_configPath);
    return true;
}

size_t TeleportManager::getGroupCount() const {
    return m_groups.size();
}

const TeleportGroup* TeleportManager::getGroup(size_t index) const {
    if (index >= m_groups.size()) {
        return nullptr;
    }
    return &m_groups[index];
}

TeleportGroup* TeleportManager::getGroup(size_t index) {
    if (index >= m_groups.size()) {
        return nullptr;
    }
    return &m_groups[index];
}

bool TeleportManager::addTeleport(size_t groupIndex, const Teleport& teleport) {
    if (groupIndex >= m_groups.size()) {
        return false;
    }

    m_groups[groupIndex].teleports.push_back(teleport);
    saveToFile(m_configPath);
    return true;
}

bool TeleportManager::deleteTeleport(size_t groupIndex, size_t teleportIndex) {
    if (groupIndex >= m_groups.size()) {
        return false;
    }

    auto& group = m_groups[groupIndex];
    if (teleportIndex >= group.teleports.size()) {
        return false;
    }

    group.teleports.erase(group.teleports.begin() + teleportIndex);
    saveToFile(m_configPath);
    return true;
}

bool TeleportManager::updateTeleport(size_t groupIndex, size_t teleportIndex, const Teleport& teleport) {
    if (groupIndex >= m_groups.size()) {
        return false;
    }

    auto& group = m_groups[groupIndex];
    if (teleportIndex >= group.teleports.size()) {
        return false;
    }

    group.teleports[teleportIndex] = teleport;
    saveToFile(m_configPath);
    return true;
}

std::string TeleportManager::generateUniqueTeleportName(size_t groupIndex) const {
    if (groupIndex >= m_groups.size()) {
        return "Teleport 1";
    }

    const auto& group = m_groups[groupIndex];
    int number = 1;
    std::string baseName = "Teleport ";
    
    while (true) {
        std::string candidateName = baseName + std::to_string(number);
        bool exists = false;

        for (const auto& tp : group.teleports) {
            if (tp.name == candidateName) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            return candidateName;
        }
        number++;
    }
}

bool TeleportManager::loadFromFile(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            StatusUI::AddMessage("WARN: Could not open teleports config file: " + filepath);
            return false;
        }

        json j;
        file >> j;
        file.close();

        if (!j.contains("teleport_groups") || !j["teleport_groups"].is_array()) {
            StatusUI::AddMessage("ERROR: Invalid teleports config format");
            return false;
        }

        m_groups.clear();

        for (const auto& groupJson : j["teleport_groups"]) {
            if (!groupJson.contains("name") || !groupJson.contains("teleports")) {
                continue;
            }

            TeleportGroup group(groupJson["name"].get<std::string>());

            for (const auto& tpJson : groupJson["teleports"]) {
                if (!tpJson.contains("name") || !tpJson.contains("coordinates") || !tpJson.contains("map")) {
                    continue;
                }

                auto coords = tpJson["coordinates"];
                if (!coords.is_array() || coords.size() != 3) {
                    continue;
                }

                Teleport tp(
                    tpJson["name"].get<std::string>(),
                    coords[0].get<float>(),
                    coords[1].get<float>(),
                    coords[2].get<float>(),
                    tpJson["map"].get<int>()
                );

                group.teleports.push_back(tp);
            }

            m_groups.push_back(group);
        }

        StatusUI::AddMessage("INFO: Loaded " + std::to_string(m_groups.size()) + " teleport groups");
        return true;

    } catch (const std::exception& e) {
        StatusUI::AddMessage("ERROR: Failed to load teleports config: " + std::string(e.what()));
        return false;
    }
}

bool TeleportManager::saveToFile(const std::string& filepath) const {
    try {
        json j;
        json groupsArray = json::array();

        for (const auto& group : m_groups) {
            json groupJson;
            groupJson["name"] = group.name;
            
            json teleportsArray = json::array();
            for (const auto& tp : group.teleports) {
                json tpJson;
                tpJson["name"] = tp.name;
                tpJson["coordinates"] = json::array({ tp.coordinates[0], tp.coordinates[1], tp.coordinates[2] });
                tpJson["map"] = tp.mapId;
                teleportsArray.push_back(tpJson);
            }

            groupJson["teleports"] = teleportsArray;
            groupsArray.push_back(groupJson);
        }

        j["teleport_groups"] = groupsArray;

        std::ofstream file(filepath);
        if (!file.is_open()) {
            StatusUI::AddMessage("ERROR: Could not open file for writing: " + filepath);
            return false;
        }

        file << j.dump(2);
        file.close();
        return true;

    } catch (const std::exception& e) {
        StatusUI::AddMessage("ERROR: Failed to save teleports config: " + std::string(e.what()));
        return false;
    }
}

bool TeleportManager::importFromJson(const std::string& jsonContent) {
    try {
        json j = json::parse(jsonContent);

        if (!j.contains("teleport_groups") || !j["teleport_groups"].is_array()) {
            StatusUI::AddMessage("ERROR: Invalid JSON format for import");
            return false;
        }

        for (const auto& groupJson : j["teleport_groups"]) {
            if (!groupJson.contains("name") || !groupJson.contains("teleports")) {
                StatusUI::AddMessage("ERROR: Invalid group format in import");
                continue;
            }

            TeleportGroup group(groupJson["name"].get<std::string>());

            for (const auto& tpJson : groupJson["teleports"]) {
                if (!tpJson.contains("name") || !tpJson.contains("coordinates") || !tpJson.contains("map")) {
                    continue;
                }

                auto coords = tpJson["coordinates"];
                if (!coords.is_array() || coords.size() != 3) {
                    continue;
                }

                Teleport tp(
                    tpJson["name"].get<std::string>(),
                    coords[0].get<float>(),
                    coords[1].get<float>(),
                    coords[2].get<float>(),
                    tpJson["map"].get<int>()
                );

                group.teleports.push_back(tp);
            }

            m_groups.push_back(group);
        }

        saveToFile(m_configPath);
        StatusUI::AddMessage("INFO: Import successful");
        return true;

    } catch (const std::exception& e) {
        StatusUI::AddMessage("ERROR: Failed to import JSON: " + std::string(e.what()));
        return false;
    }
}

bool TeleportManager::importFromJsonScaled(const std::string& jsonContent, float scaleFactor) {
    try {
        json j = json::parse(jsonContent);

        if (!j.contains("teleport_groups") || !j["teleport_groups"].is_array()) {
            StatusUI::AddMessage("ERROR: Invalid JSON format for import");
            return false;
        }

        for (const auto& groupJson : j["teleport_groups"]) {
            if (!groupJson.contains("name") || !groupJson.contains("teleports")) {
                StatusUI::AddMessage("ERROR: Invalid group format in import");
                continue;
            }

            TeleportGroup group(groupJson["name"].get<std::string>());

            for (const auto& tpJson : groupJson["teleports"]) {
                if (!tpJson.contains("name") || !tpJson.contains("coordinates") || !tpJson.contains("map")) {
                    continue;
                }

                auto coords = tpJson["coordinates"];
                if (!coords.is_array() || coords.size() != 3) {
                    continue;
                }

                Teleport tp(
                    tpJson["name"].get<std::string>(),
                    static_cast<float>(coords[0].get<double>() * static_cast<double>(scaleFactor)),
                    static_cast<float>(coords[1].get<double>() * static_cast<double>(scaleFactor)),
                    static_cast<float>(coords[2].get<double>() * static_cast<double>(scaleFactor)),
                    tpJson["map"].get<int>()
                );

                group.teleports.push_back(tp);
            }

            m_groups.push_back(group);
        }

        saveToFile(m_configPath);
        StatusUI::AddMessage("INFO: Import successful (scale: " + std::to_string(scaleFactor) + "x)");
        return true;

    } catch (const std::exception& e) {
        StatusUI::AddMessage("ERROR: Failed to import JSON: " + std::string(e.what()));
        return false;
    }
}

std::string TeleportManager::exportToJson(size_t groupIndex) const {
    if (groupIndex >= m_groups.size()) {
        return "{}";
    }

    try {
        const auto& group = m_groups[groupIndex];
        json j;
        json groupsArray = json::array();
        json groupJson;

        groupJson["name"] = group.name;
        json teleportsArray = json::array();

        for (const auto& tp : group.teleports) {
            json tpJson;
            tpJson["name"] = tp.name;
            tpJson["coordinates"] = json::array({ tp.coordinates[0], tp.coordinates[1], tp.coordinates[2] });
            tpJson["map"] = tp.mapId;
            teleportsArray.push_back(tpJson);
        }

        groupJson["teleports"] = teleportsArray;
        groupsArray.push_back(groupJson);
        j["teleport_groups"] = groupsArray;

        return j.dump(2);

    } catch (const std::exception& e) {
        StatusUI::AddMessage("ERROR: Failed to export group: " + std::string(e.what()));
        return "{}";
    }
}
