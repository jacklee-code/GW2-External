#pragma once

#include <string>
#include <vector>
#include <functional>
#include "hotkey_definitions.h"
#include "teleport_manager.h"

class Hack;

class HackGUI {
public:
    HackGUI(Hack& hack);
    bool renderUI();

private:
    Hack& m_hack;

    // Only GUI-specific state or user preferences here
    bool m_sprintEnabled = false; // User's preference toggle for sprint mode

    // Refactored Hotkey Management
    std::vector<HotkeyInfo> m_hotkeys;
    HotkeyID m_rebinding_hotkey_id = HotkeyID::NONE; // ID of the hotkey currently being rebound

    // Teleport Management
    TeleportManager m_teleportManager;
    int m_selectedGroupIndex = 0;
    int m_editingTeleportIndex = -1;
    bool m_showEditTeleportWindow = false;
    bool m_showDeleteGroupConfirm = false;
    bool m_showDeleteTeleportConfirm = false;
    bool m_focusNextWindow = false; // Set focus only once when opening a sub-window
    int m_deleteGroupIndex = -1;
    int m_deleteTeleportIndex = -1;
    int m_deleteTeleportGroupIndex = -1; // Track which group the teleport belongs to
    bool m_showAddGroupWindow = false;
    bool m_showRenameGroupWindow = false;
    
    // Edit window state
    char m_editTeleportName[128] = "";
    float m_editTeleportCoords[3] = { 0.0f, 0.0f, 0.0f };
    int m_editTeleportMapId = -1;
    
    // Add/Rename group state
    char m_newGroupName[128] = "";
    
    // Import scale state
    bool m_showImportScaleWindow = false;
    std::string m_importJsonContent;
    std::string m_importFilePath;
    float m_importScaleFactor = 1.0f;
    int m_importScalePreset = 0; // 0=Custom

    // UI Rendering Methods
    void RenderAlwaysOnTop();
    void RenderTogglesSection();
    void RenderHotkeysSection();
    void RenderLogSection();
    void RenderInfoSection();
    void RenderTeleportsTab();
    void RenderEditTeleportWindow();
    void RenderAddGroupWindow();
    void RenderRenameGroupWindow();
    void RenderDeleteGroupConfirmWindow();
    void RenderDeleteTeleportConfirmWindow();
    void RenderImportScaleWindow();

    // Logic Handling Methods
    void HandleHotkeys();
    void HandleHotkeyRebinding();

    // Helpers
    void RenderHotkeyControl(HotkeyInfo& hotkey);
};