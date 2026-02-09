#pragma once

#include "imgui/imgui.h"
#include <windows.h>
#include <string>
#include <array>

namespace UIHelpers {
    // Load icon from resources
    bool LoadTextureFromResource(int resourceId, ImTextureID* out_texture, int* out_width, int* out_height);
    
    // Open file dialog for JSON import
    std::string OpenFileDialog(const char* filter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0");
    
    // Win32 input dialog - shows a modal dialog with a single text input field.
    // Returns true if user confirmed, false if cancelled. Result stored in outText.
    bool ShowInputDialog(const char* title, const char* prompt, std::string& outText, const std::string& defaultText = "");
    
    // Win32 edit teleport dialog - shows a modal dialog for editing teleport properties.
    // Returns true if user confirmed. All out parameters are updated on confirm.
    struct EditTeleportResult {
        char name[128] = "";
        float coords[3] = { 0.0f, 0.0f, 0.0f };
        int mapId = -1;
        bool setCurrentPos = false; // Set to true if user clicked "Set Current Position"
    };
    bool ShowEditTeleportDialog(const char* title, EditTeleportResult& data);
}
