#pragma once

#include "imgui/imgui.h"
#include <windows.h>
#include <string>

namespace UIHelpers {
    // Load icon from resources
    bool LoadTextureFromResource(int resourceId, ImTextureID* out_texture, int* out_width, int* out_height);
    
    // Open file dialog for JSON import
    std::string OpenFileDialog(const char* filter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0");
}
