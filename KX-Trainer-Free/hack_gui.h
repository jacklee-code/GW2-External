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
    bool IsWindowCollapsed() const { return m_windowCollapsed; }

private:
    Hack& m_hack;

    // Only GUI-specific state or user preferences here
    bool m_sprintEnabled = false; // User's preference toggle for sprint mode
    bool m_windowCollapsed = false;  // Window collapsed state (true = collapsed to title bar only)

    // Refactored Hotkey Management
    std::vector<HotkeyInfo> m_hotkeys;
    HotkeyID m_rebinding_hotkey_id = HotkeyID::NONE; // ID of the hotkey currently being rebound

    // Teleport Management
    TeleportManager m_teleportManager;
    int m_selectedGroupIndex = 0;
    int m_editingTeleportIndex = -1;
    
    // Edit window state (used for Set Current Position loop)
    char m_editTeleportName[128] = "";
    float m_editTeleportCoords[3] = { 0.0f, 0.0f, 0.0f };
    int m_editTeleportMapId = -1;
    
    // Add/Rename group state
    char m_newGroupName[128] = "";
    
    // Import scale state (only popup still using ImGui modal - triggered after file dialog so focus is guaranteed)
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
    void RenderImportScalePopup();

    // Logic Handling Methods
    void HandleHotkeys();
    void HandleHotkeyRebinding();

    // Hotkey Configuration
    void SaveHotkeyConfig();
    void LoadHotkeyConfig();

    // Helpers
    void RenderHotkeyControl(HotkeyInfo& hotkey);
};