#include "hack_gui.h"
#include "hack.h"
#include "constants.h"
#include "status_ui.h"
#include "key_utils.h"
#include "ui_helpers.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <sstream>

HackGUI::HackGUI(Hack& hack) : m_hack(hack), m_rebinding_hotkey_id(HotkeyID::NONE) {
    // Define all available hotkeys and their default properties
    m_hotkeys = {
        {HotkeyID::SAVE_POS,             "Save Position",   Constants::Hotkeys::KEY_SAVEPOS,         HotkeyTriggerType::ON_PRESS, [](Hack& h, bool) { h.savePosition(); }},
        {HotkeyID::LOAD_POS,             "Load Position",   Constants::Hotkeys::KEY_LOADPOS,         HotkeyTriggerType::ON_PRESS, [](Hack& h, bool) { h.loadPosition(); }},
        {HotkeyID::TOGGLE_INVISIBILITY,  "Invisibility",    Constants::Hotkeys::KEY_INVISIBILITY,    HotkeyTriggerType::ON_PRESS, [](Hack& h, bool) { h.toggleInvisibility(!h.IsInvisibilityEnabled()); }},
        {HotkeyID::TOGGLE_WALLCLIMB,     "Wall Climb",      Constants::Hotkeys::KEY_WALLCLIMB,       HotkeyTriggerType::ON_PRESS, [](Hack& h, bool) { h.toggleWallClimb(!h.IsWallClimbEnabled()); }},
        {HotkeyID::TOGGLE_CLIPPING,      "Clipping",        Constants::Hotkeys::KEY_CLIPPING,        HotkeyTriggerType::ON_PRESS, [](Hack& h, bool) { h.toggleClipping(!h.IsClippingEnabled()); }},
        {HotkeyID::TOGGLE_OBJECT_CLIPPING,"Object Clipping", Constants::Hotkeys::KEY_OBJECT_CLIPPING, HotkeyTriggerType::ON_PRESS, [](Hack& h, bool) { h.toggleObjectClipping(!h.IsObjectClippingEnabled()); }},
        {HotkeyID::TOGGLE_FULL_STRAFE,   "Full Strafe",     Constants::Hotkeys::KEY_FULL_STRAFE,     HotkeyTriggerType::ON_PRESS, [](Hack& h, bool) { h.toggleFullStrafe(!h.IsFullStrafeEnabled()); }},
        {HotkeyID::TOGGLE_NO_FOG,        "No Fog",          Constants::Hotkeys::KEY_NO_FOG,          HotkeyTriggerType::ON_PRESS, [](Hack& h, bool) { h.toggleFog(!h.IsFogEnabled()); }},
        {HotkeyID::HOLD_SUPER_SPRINT,    "Super Sprint",    Constants::Hotkeys::KEY_SUPER_SPRINT,    HotkeyTriggerType::ON_HOLD,  [](Hack& h, bool held) { h.handleSuperSprint(held); }},
        {HotkeyID::TOGGLE_SPRINT_PREF,   "Sprint",          Constants::Hotkeys::KEY_SPRINT,          HotkeyTriggerType::ON_PRESS, [this](Hack& /*h*/, bool) { this->m_sprintEnabled = !this->m_sprintEnabled; }}, // Toggles the GUI preference flag
        {HotkeyID::HOLD_FLY,             "Fly",             Constants::Hotkeys::KEY_FLY,             HotkeyTriggerType::ON_HOLD,  [](Hack& h, bool held) { h.handleFly(held); }}
    };

    // TODO: Load saved currentKeyCode values from a config file here, overwriting the defaults set in HotkeyInfo constructor
}

// Renders label, key name, and Change button for a single hotkey configuration
void HackGUI::RenderHotkeyControl(HotkeyInfo& hotkey) {
    ImGui::Text("%s:", hotkey.name);
    ImGui::SameLine(150.0f); // Alignment

    if (m_rebinding_hotkey_id == hotkey.id) {
        ImGui::TextDisabled("<Press any key>");
    } else {
        // Display key name, adding '*' for hold actions
        const char* baseKeyName = GetKeyName(hotkey.currentKeyCode);
        if (hotkey.triggerType == HotkeyTriggerType::ON_HOLD) {
            std::string keyNameWithIndicator = std::string(baseKeyName) + "*";
            ImGui::Text("%s", keyNameWithIndicator.c_str());
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Hold");
            }
        } else {
            ImGui::Text("%s", baseKeyName);
        }
        ImGui::SameLine(280.0f); // Alignment

        // Create a unique ID for the button using the hotkey name
        std::string button_label = "Change##" + std::string(hotkey.name);
        if (ImGui::Button(button_label.c_str())) {
            m_rebinding_hotkey_id = hotkey.id; // Set the ID of the hotkey being rebound
        }
    }
}

// Renders the "Always on Top" checkbox and applies the setting
void HackGUI::RenderAlwaysOnTop() {
    static bool always_on_top_checkbox = false;
    static bool current_window_is_topmost = false;
    HWND current_window_hwnd = nullptr;

    ImGuiWindow* current_imgui_win = ImGui::GetCurrentWindowRead();
    if (current_imgui_win && current_imgui_win->Viewport) {
        current_window_hwnd = (HWND)current_imgui_win->Viewport->PlatformHandleRaw;
    }

    ImGui::Checkbox("Always on Top", &always_on_top_checkbox);

    if (current_window_hwnd) {
        HWND insert_after = always_on_top_checkbox ? HWND_TOPMOST : HWND_NOTOPMOST;
        bool should_be_topmost = always_on_top_checkbox;

        if (should_be_topmost != current_window_is_topmost) {
            ::SetWindowPos(current_window_hwnd, insert_after, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            current_window_is_topmost = should_be_topmost;
        }
    }
    ImGui::Separator();
    ImGui::Spacing();
}

// Checks registered hotkeys and calls corresponding actions
void HackGUI::HandleHotkeys() {
    // Don't process hotkeys if currently rebinding one
    if (m_rebinding_hotkey_id != HotkeyID::NONE) {
        return;
    }

    for (const auto& hotkey : m_hotkeys) {
        if (hotkey.currentKeyCode == 0 || !hotkey.action) { // Skip unbound or unassigned actions
            continue;
        }

        SHORT keyState = GetAsyncKeyState(hotkey.currentKeyCode);

        switch (hotkey.triggerType) {
            case HotkeyTriggerType::ON_PRESS:
                // Check the least significant bit (1 means pressed *since the last call*)
                if (keyState & 1) {
                    hotkey.action(m_hack, true); // Pass true for pressed state
                }
                break;

            case HotkeyTriggerType::ON_HOLD:
                // Check the most significant bit (1 means currently held down)
                bool isHeld = (keyState & 0x8000) != 0;
                hotkey.action(m_hack, isHeld); // Pass the current hold state
                break;
        }
    }
}

// Handles the logic when the user is actively rebinding a hotkey
void HackGUI::HandleHotkeyRebinding() {
    if (m_rebinding_hotkey_id == HotkeyID::NONE) return;

    int captured_vk = -1; // -1 means no key captured yet

    // Check special keys first
    if (GetAsyncKeyState(VK_ESCAPE) & 1) {
        captured_vk = VK_ESCAPE; // Special value to indicate cancellation
    } else if ((GetAsyncKeyState(VK_DELETE) & 1) || (GetAsyncKeyState(VK_BACK) & 1)) {
        captured_vk = 0; // 0 means unbind
    } else {
        // Iterate through common key codes to find the first pressed key
        for (int vk = VK_MBUTTON; vk < VK_OEM_CLEAR; ++vk) {
            // Skip keys that interfere, are unsuitable, or handled above
            if (vk == VK_ESCAPE || vk == VK_DELETE || vk == VK_BACK ||
                vk == VK_LBUTTON || vk == VK_RBUTTON || // Avoid UI clicks binding easily
                vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU || // Prefer specific L/R versions if needed, though GetKeyName handles these
                vk == VK_CAPITAL || vk == VK_NUMLOCK || vk == VK_SCROLL) // State keys are poor choices
            {
                continue;
            }
            // Check if key was pressed *since the last call*
            if (GetAsyncKeyState(vk) & 1) {
                captured_vk = vk;
                break; // Found the first pressed key
            }
        }
    }

    // If a key was captured or an action key (Esc/Del/Back) was pressed
    if (captured_vk != -1) {
        if (captured_vk != VK_ESCAPE) { // If not cancelling
            // Find the hotkey being rebound in the vector
            for (auto& hotkey : m_hotkeys) {
                if (hotkey.id == m_rebinding_hotkey_id) {
                    hotkey.currentKeyCode = captured_vk; // Assign the captured key (or 0 for unbind)
                    // TODO: Save updated hotkeys to config file here
                    break; // Found and updated
                }
            }
        }
        // Reset rebinding state regardless of whether we assigned or cancelled
        m_rebinding_hotkey_id = HotkeyID::NONE;
    }
}

// Renders the collapsible section with toggle checkboxes
void HackGUI::RenderTogglesSection() {
    if (ImGui::CollapsingHeader("Toggles", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool tempState = false; // Temporary variable for ImGui interaction

        tempState = m_hack.IsFogEnabled();
        if (ImGui::Checkbox("No Fog", &tempState)) { m_hack.toggleFog(tempState); }

        tempState = m_hack.IsObjectClippingEnabled();
        if (ImGui::Checkbox("Object Clipping", &tempState)) { m_hack.toggleObjectClipping(tempState); }

        tempState = m_hack.IsFullStrafeEnabled();
        if (ImGui::Checkbox("Full Strafe", &tempState)) { m_hack.toggleFullStrafe(tempState); }

        // Sprint checkbox controls the user preference flag
        ImGui::Checkbox("Sprint", &m_sprintEnabled);

        tempState = m_hack.IsInvisibilityEnabled();
        if (ImGui::Checkbox("Invisibility (Mobs)", &tempState)) { m_hack.toggleInvisibility(tempState); }

        tempState = m_hack.IsWallClimbEnabled();
        if (ImGui::Checkbox("Wall Climb", &tempState)) { m_hack.toggleWallClimb(tempState); }

        tempState = m_hack.IsClippingEnabled();
        if (ImGui::Checkbox("Clipping", &tempState)) { m_hack.toggleClipping(tempState); }

        ImGui::Spacing();
    }
}

// Renders the collapsible section for configuring hotkeys
void HackGUI::RenderHotkeysSection() {
    if (ImGui::CollapsingHeader("Hotkeys")) {
        // Display rebinding prompt if active
        if (m_rebinding_hotkey_id != HotkeyID::NONE) {
            const char* rebinding_name = "Unknown"; // Fallback
            for(const auto& hk : m_hotkeys) {
                if (hk.id == m_rebinding_hotkey_id) {
                    rebinding_name = hk.name;
                    break;
                }
            }
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Rebinding '%s'. Press a key (ESC to cancel, DEL/BKSP to clear)...", rebinding_name);
            ImGui::Separator();
        }

        // Dim controls while rebinding
        bool disable_controls = (m_rebinding_hotkey_id != HotkeyID::NONE);
        if (disable_controls) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        // Iterate through hotkeys and render controls
        for (auto& hotkey : m_hotkeys) {
            RenderHotkeyControl(hotkey);
        }


        if (disable_controls) {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }
        ImGui::Separator();
        ImGui::Spacing();

        // Buttons for defaults and unbinding
        if (ImGui::Button("Apply Recommended Defaults")) {
            for (auto& hotkey : m_hotkeys) {
                hotkey.currentKeyCode = hotkey.defaultKeyCode;
            }
            // TODO: Save updated hotkeys to config file
        }
        ImGui::SameLine();
        if (ImGui::Button("Unbind All")) {
            for (auto& hotkey : m_hotkeys) {
                hotkey.currentKeyCode = 0; // 0 represents unbound
            }
            // TODO: Save updated hotkeys to config file
        }
        ImGui::Spacing();
    }
}

// Renders the collapsible log section
void HackGUI::RenderLogSection() {
    // Assumes StatusUI is still used for logging until Step 1 is done.
    // If Step 1 (ILogger) was done, this would pull from the logger instance.
    if (ImGui::CollapsingHeader("Log", ImGuiTreeNodeFlags_None)) { // Start collapsed
        ImGui::BeginChild("LogScrollingRegion", ImVec2(0, 100), true, ImGuiWindowFlags_HorizontalScrollbar);
        {
            std::vector<std::string> current_messages = StatusUI::GetMessages(); // <-- Keep using StatusUI for now

            for (const auto& msg : current_messages) {
                ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Text); // Default
                if (msg.rfind("ERROR:", 0) == 0) color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); // Red
                else if (msg.rfind("WARN:", 0) == 0) color = ImVec4(1.0f, 1.0f, 0.4f, 1.0f); // Yellow
                else if (msg.rfind("INFO:", 0) == 0) color = ImVec4(0.5f, 1.0f, 0.5f, 1.0f); // Green
                ImGui::TextColored(color, "%s", msg.c_str());
            }
            // Auto-scroll
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - ImGui::GetTextLineHeight() * 2) {
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();

        if (ImGui::Button("Clear Log")) {
            StatusUI::ClearMessages(); // <-- Keep using StatusUI for now
        }
        ImGui::Spacing();
    }
}

// Renders the collapsible info/about section
void HackGUI::RenderInfoSection() {
if (ImGui::CollapsingHeader("Info")) {
    ImGui::Text("Jack's GW2 External");
    ImGui::Text("Modified by Jack Lee");
        ImGui::Separator();

        // GitHub Link
        ImGui::Text("GitHub:");
        ImGui::SameLine();
        if (ImGui::Button("Repository")) {
            ShellExecuteA(NULL, "open", "https://github.com/Krixx1337/KX-Trainer-Free", NULL, NULL, SW_SHOWNORMAL);
        }

        // kxtools.xyz Link
        ImGui::Text("Website:");
        ImGui::SameLine();
        if (ImGui::Button("kxtools.xyz")) {
             ShellExecuteA(NULL, "open", "https://kxtools.xyz", NULL, NULL, SW_SHOWNORMAL);
        }

        // Discord Link
        ImGui::Text("Discord:");
        ImGui::SameLine();
        if (ImGui::Button("Join Server")) {
             ShellExecuteA(NULL, "open", "https://discord.gg/z92rnB4kHm", NULL, NULL, SW_SHOWNORMAL);
        }
    }
}

// Main render function for the HackGUI window
bool HackGUI::renderUI()
{
    static bool main_window_open = true;
    static bool first_frame = true;
    bool exit_requested = false;

    // Set initial window size on first frame
    if (first_frame) {
        ImGui::SetNextWindowSize(ImVec2(600.0f, 500.0f), ImGuiCond_FirstUseEver);
        first_frame = false;
    }

    const float min_window_width = 550.0f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(min_window_width, 300.0f), ImVec2(FLT_MAX, FLT_MAX));

    ImGuiWindowFlags window_flags = 0;
    ImGui::Begin("Jack's GW2 External", &main_window_open, window_flags);

    if (!main_window_open) {
        exit_requested = true; // Request exit if user closes window
    }

    RenderAlwaysOnTop();

    m_hack.refreshAddresses(); // Ensure pointers are valid before reading/writing
    HandleHotkeys();           // Process registered hotkeys
    HandleHotkeyRebinding();   // Handle input if rebinding

    // Apply continuous states based on user preference toggles
    m_hack.handleSprint(m_sprintEnabled);

    // Render UI sections in tabs
    if (ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_None)) {
        // Core Tab
        if (ImGui::BeginTabItem("Core")) {
            RenderTogglesSection();
            RenderHotkeysSection();
            RenderLogSection();
            RenderInfoSection();
            ImGui::EndTabItem();
        }

        // Teleports Tab
        if (ImGui::BeginTabItem("Teleports")) {
            RenderTeleportsTab();
            ImGui::EndTabItem();
        }

    ImGui::EndTabBar();
    }

    // Render import scale popup (only remaining ImGui popup modal)
    RenderImportScalePopup();

    ImGui::End();

    return exit_requested;
}

void HackGUI::RenderTeleportsTab() {
    // Group selection dropdown at the bottom
    ImGui::BeginChild("TeleportList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2), true);
    
    const TeleportGroup* currentGroup = m_teleportManager.getGroup(m_selectedGroupIndex);
    if (currentGroup) {
        const float editBtnWidth = 50.0f;
        const float deleteBtnWidth = 30.0f;
        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        
        for (size_t i = 0; i < currentGroup->teleports.size(); ++i) {
            const auto& tp = currentGroup->teleports[i];
            
            // Calculate teleport button width (fill remaining space)
            float availWidth = ImGui::GetContentRegionAvail().x;
            float teleportBtnWidth = availWidth - editBtnWidth - deleteBtnWidth - (spacing * 2);
            
            // Teleport button (main area)
            if (ImGui::Button(tp.name.c_str(), ImVec2(teleportBtnWidth, 0))) {
                // Teleport to this position
                m_hack.writeXYZ(tp.coordinates[0], tp.coordinates[1], tp.coordinates[2]);
                StatusUI::AddMessage("INFO: Teleported to " + tp.name);
            }
            
            ImGui::SameLine();
            
            // Edit button
            std::string editLabel = "Edit##" + std::to_string(i);
            if (ImGui::Button(editLabel.c_str(), ImVec2(editBtnWidth, 0))) {
                UIHelpers::EditTeleportResult editData = {};
                strcpy_s(editData.name, tp.name.c_str());
                editData.coords[0] = tp.coordinates[0];
                editData.coords[1] = tp.coordinates[1];
                editData.coords[2] = tp.coordinates[2];
                editData.mapId = tp.mapId;
                // Loop to handle "Set Current Position" re-opening
                bool keepEditing = true;
                while (keepEditing) {
                    if (UIHelpers::ShowEditTeleportDialog("Edit Teleport", editData)) {
                        if (editData.setCurrentPos) {
                            m_hack.refreshAddresses();
                            m_hack.readXYZ();
                            editData.coords[0] = m_hack.m_xValue;
                            editData.coords[1] = m_hack.m_yValue;
                            editData.coords[2] = m_hack.m_zValue;
                            editData.setCurrentPos = false;
                            continue; // Re-open dialog with updated coords
                        }
                        Teleport updatedTp(editData.name, editData.coords[0], editData.coords[1], editData.coords[2], editData.mapId);
                        if (m_teleportManager.updateTeleport(m_selectedGroupIndex, static_cast<int>(i), updatedTp)) {
                            StatusUI::AddMessage("INFO: Teleport updated");
                        }
                    }
                    keepEditing = false;
                }
            }
            
            ImGui::SameLine();
            
            // Delete button - Using MessageBox for reliable confirmation
            std::string deleteLabel = "X##tp_" + std::to_string(m_selectedGroupIndex) + "_" + std::to_string(i);
            if (ImGui::Button(deleteLabel.c_str(), ImVec2(deleteBtnWidth, 0))) {
                std::string message = "Delete teleport '" + tp.name + "'?";
                int result = MessageBoxA(NULL, message.c_str(), "Confirm Delete Teleport", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL);
                if (result == IDOK) {
                    if (m_teleportManager.deleteTeleport(m_selectedGroupIndex, i)) {
                        StatusUI::AddMessage("INFO: Teleport deleted");
                    }
                }
            }
        }
    }
    
    ImGui::EndChild();
    
    // Bottom control bar
    ImGui::Separator();
    
    // Calculate widths for smart layout
    const float availWidth = ImGui::GetContentRegionAvail().x;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const float iconBtnWidth = 30.0f;  // Width for icon buttons
    const float minComboWidth = 120.0f;  // Minimum width for combo
    const float minAddTeleportWidth = 100.0f;  // Minimum width for "Add Teleport"
    
    // Calculate minimum required width
    const float minRequiredWidth = minComboWidth + spacing + 
                                   iconBtnWidth + spacing +  // Add Group
                                   iconBtnWidth + spacing +  // Rename
                                   iconBtnWidth + spacing +  // Delete
                                   minAddTeleportWidth + spacing + 
                                   iconBtnWidth;  // Import
    
    // Distribute extra space between combo and Add Teleport button
    float comboWidth = minComboWidth;
    float addTeleportWidth = minAddTeleportWidth;
    
    if (availWidth > minRequiredWidth) {
        float extraSpace = availWidth - minRequiredWidth;
        // Give 60% to combo, 40% to Add Teleport
        comboWidth += extraSpace * 0.6f;
        addTeleportWidth += extraSpace * 0.4f;
    }
    
    // Group dropdown
    if (currentGroup) {
        ImGui::SetNextItemWidth(comboWidth);
        if (ImGui::BeginCombo("##GroupSelect", currentGroup->name.c_str())) {
            for (size_t i = 0; i < m_teleportManager.getGroupCount(); ++i) {
                const TeleportGroup* group = m_teleportManager.getGroup(i);
                if (group) {
                    bool isSelected = (m_selectedGroupIndex == static_cast<int>(i));
                    if (ImGui::Selectable(group->name.c_str(), isSelected)) {
                        m_selectedGroupIndex = static_cast<int>(i);
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
            }
            ImGui::EndCombo();
        }
    }
    
    ImGui::SameLine();
    
    // Add Group button
    if (ImGui::Button(u8"\u2795##AddGroup", ImVec2(iconBtnWidth, 0))) {  // ➕
        std::string groupName;
        if (UIHelpers::ShowInputDialog("Add Group", "Enter group name:", groupName)) {
            if (m_teleportManager.addGroup(groupName)) {
                m_selectedGroupIndex = static_cast<int>(m_teleportManager.getGroupCount() - 1);
                StatusUI::AddMessage("INFO: Group added: " + groupName);
            }
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Add Group");
    }
    
    ImGui::SameLine();
    
    // Rename Group button
    if (ImGui::Button(u8"\u270E##RenameGroup", ImVec2(iconBtnWidth, 0))) {  // ✎
        if (currentGroup) {
            std::string newName;
            if (UIHelpers::ShowInputDialog("Rename Group", "Enter new group name:", newName, currentGroup->name)) {
                if (m_teleportManager.renameGroup(m_selectedGroupIndex, newName)) {
                    StatusUI::AddMessage("INFO: Group renamed to: " + newName);
                }
            }
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Rename Group");
    }
    
    ImGui::SameLine();
    
    // Delete Group button
    if (ImGui::Button(u8"\u274C##DeleteGroup", ImVec2(iconBtnWidth, 0))) {  // ❌
        const TeleportGroup* group = m_teleportManager.getGroup(m_selectedGroupIndex);
        if (group) {
            std::string message = "Delete group '" + group->name + "'?\n\nThis will delete all teleports in this group.";
            int result = MessageBoxA(NULL, message.c_str(), "Confirm Delete Group", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL);
            if (result == IDOK) {
                if (m_teleportManager.deleteGroup(m_selectedGroupIndex)) {
                    StatusUI::AddMessage("INFO: Group deleted");
                    if (m_selectedGroupIndex >= static_cast<int>(m_teleportManager.getGroupCount())) {
                        m_selectedGroupIndex = static_cast<int>(m_teleportManager.getGroupCount()) - 1;
                    }
                    if (m_selectedGroupIndex < 0) m_selectedGroupIndex = 0;
                }
            }
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Delete Group");
    }
    
    ImGui::SameLine();
    
    // Add Teleport button
    if (ImGui::Button("Add Teleport", ImVec2(addTeleportWidth, 0))) {
        // Read current game position
        m_hack.refreshAddresses();
        m_hack.readXYZ();
        
        // Create new teleport with current position
        Teleport newTp;
        newTp.name = m_teleportManager.generateUniqueTeleportName(m_selectedGroupIndex);
        newTp.coordinates[0] = m_hack.m_xValue;
        newTp.coordinates[1] = m_hack.m_yValue;
        newTp.coordinates[2] = m_hack.m_zValue;
        newTp.mapId = -1;
        
        m_teleportManager.addTeleport(m_selectedGroupIndex, newTp);
        StatusUI::AddMessage("INFO: Added teleport: " + newTp.name);
    }
    
    ImGui::SameLine();
    
    // Import JSON button
    if (ImGui::Button(u8"\u2B07##ImportJSON", ImVec2(iconBtnWidth, 0))) {  // ⬇
        std::string filepath = UIHelpers::OpenFileDialog("JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0");
        if (!filepath.empty()) {
            // Read file content
            std::ifstream file(filepath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                file.close();
                
                m_importJsonContent = buffer.str();
                m_importFilePath = filepath;
                m_importScaleFactor = 1.0f;
                m_importScalePreset = 0;
                m_showImportScaleWindow = true;
            } else {
                StatusUI::AddMessage("ERROR: Failed to open file: " + filepath);
            }
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Import JSON");
    }
}

void HackGUI::RenderImportScalePopup() {
// Import Settings is the only remaining ImGui popup modal.
// It's triggered after the file dialog returns, so the window always has focus.
if (m_showImportScaleWindow) {
    ImGui::OpenPopup("Import Settings");
    m_showImportScaleWindow = false;
}

ImVec2 center = ImGui::GetMainViewport()->GetCenter();
ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
ImGui::SetNextWindowSize(ImVec2(380, 0));
if (ImGui::BeginPopupModal("Import Settings", nullptr, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Coordinate Scale");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Scale all imported X, Y, Z coordinates.");
        ImGui::Text("Default is 1.0 (no change).");
        ImGui::Spacing();
        const char* presetLabels[] = { "Custom", "x1 (None)", "/32", "/10", "x10", "x32" };
        const float presetValues[] = { 0.0f, 1.0f, 1.0f / 32.0f, 0.1f, 10.0f, 32.0f };
        const int presetCount = 6;
        ImGui::Text("Presets:");
        ImGui::SameLine();
        for (int i = 1; i < presetCount; ++i) {
            if (i > 1) ImGui::SameLine();
            if (ImGui::Button(presetLabels[i])) {
                m_importScaleFactor = presetValues[i];
                m_importScalePreset = i;
            }
        }
        ImGui::Spacing();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputFloat("##ScaleFactor", &m_importScaleFactor, 0.01f, 1.0f, "%.6f")) {
            m_importScalePreset = 0;
        }
        if (m_importScaleFactor == 0.0f) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Scale cannot be 0!");
        }
        ImGui::Spacing();
        ImGui::Separator();
        bool canImport = (m_importScaleFactor != 0.0f);
        if (!canImport) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }
        if (ImGui::Button("Import", ImVec2(120, 0))) {
            if (m_teleportManager.importFromJsonScaled(m_importJsonContent, m_importScaleFactor)) {
                StatusUI::AddMessage("INFO: Successfully imported from " + m_importFilePath);
                if (m_selectedGroupIndex >= static_cast<int>(m_teleportManager.getGroupCount())) {
                    m_selectedGroupIndex = static_cast<int>(m_teleportManager.getGroupCount()) - 1;
                }
            }
            m_importJsonContent.clear();
            ImGui::CloseCurrentPopup();
        }
        if (!canImport) {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_importJsonContent.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}