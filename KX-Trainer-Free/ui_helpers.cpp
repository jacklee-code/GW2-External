#include "ui_helpers.h"
#include <commdlg.h>
#include <vector>

namespace UIHelpers {

    std::string OpenFileDialog(const char* filter) {
        OPENFILENAMEA ofn;
        char szFile[260] = { 0 };

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE) {
            return std::string(ofn.lpstrFile);
        }

        return "";
    }

    // Note: For actual icon loading from resources, you would need to implement
    // texture loading specific to your rendering backend (DirectX/OpenGL)
    // This is a placeholder that returns false
    bool LoadTextureFromResource(int resourceId, ImTextureID* out_texture, int* out_width, int* out_height) {
        // TODO: Implement based on your rendering backend
        // For now, we'll use Unicode symbols instead of actual images
        return false;
    }
}
