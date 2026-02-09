#include "ui_helpers.h"
#include <commdlg.h>
#include <vector>
#include <cstring>
#include <cstdio>

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

    bool LoadTextureFromResource(int resourceId, ImTextureID* out_texture, int* out_width, int* out_height) {
        return false;
    }

    // --- Win32 Input Dialog ---
    // Uses DialogBoxIndirectParam to create a runtime dialog template (no .rc resource needed).

    struct InputDialogData {
        const char* prompt;
        char buffer[128];
    };

    static INT_PTR CALLBACK InputDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_INITDIALOG: {
            InputDialogData* data = reinterpret_cast<InputDialogData*>(lParam);
            ::SetWindowLongPtrA(hDlg, GWLP_USERDATA, (LONG_PTR)data);
            ::SetDlgItemTextA(hDlg, 10, data->prompt);
            ::SetDlgItemTextA(hDlg, 11, data->buffer);
            // Select all text in edit box
            ::SendDlgItemMessageA(hDlg, 11, EM_SETSEL, 0, -1);
            ::SetFocus(::GetDlgItem(hDlg, 11));
            return FALSE; // We set focus manually
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                InputDialogData* data = reinterpret_cast<InputDialogData*>(::GetWindowLongPtrA(hDlg, GWLP_USERDATA));
                ::GetDlgItemTextA(hDlg, 11, data->buffer, sizeof(data->buffer));
                if (strlen(data->buffer) == 0) {
                    ::MessageBeep(MB_ICONEXCLAMATION);
                    ::SetFocus(::GetDlgItem(hDlg, 11));
                    return TRUE;
                }
                ::EndDialog(hDlg, IDOK);
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) {
                ::EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
        case WM_CLOSE:
            ::EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        return FALSE;
    }

    // Helper to build a dialog template in memory
    static std::vector<BYTE> BuildInputDialogTemplate(const char* title) {
        // Dialog template structure built in memory
        // This avoids needing .rc resource files
        std::vector<BYTE> buf;
        buf.reserve(1024);

        auto align = [&](int boundary) {
            while (buf.size() % boundary != 0) buf.push_back(0);
        };
        auto pushWord = [&](WORD w) {
            buf.push_back((BYTE)(w & 0xFF));
            buf.push_back((BYTE)((w >> 8) & 0xFF));
        };
        auto pushDword = [&](DWORD dw) {
            pushWord((WORD)(dw & 0xFFFF));
            pushWord((WORD)((dw >> 16) & 0xFFFF));
        };
        auto pushStringW = [&](const wchar_t* s) {
            for (; *s; s++) pushWord((WORD)*s);
            pushWord(0);
        };
        auto pushStringA = [&](const char* s) {
            while (*s) { pushWord((WORD)(unsigned char)*s); s++; }
            pushWord(0);
        };

        // DLGTEMPLATE
        DWORD style = DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_SETFONT;
        pushDword(style);         // style
        pushDword(0);             // dwExtendedStyle
        pushWord(4);              // cdit (number of items: label, edit, OK, Cancel)
        pushWord(0);              // x
        pushWord(0);              // y
        pushWord(180);            // cx
        pushWord(70);             // cy
        pushWord(0);              // menu
        pushWord(0);              // class
        pushStringA(title);       // title
        // DS_SETFONT: font size and name
        pushWord(9);              // font size
        pushStringW(L"Segoe UI"); // font name

        // Control 1: Static label (ID=10)
        align(4);
        pushDword(WS_CHILD | WS_VISIBLE | SS_LEFT); // style
        pushDword(0);             // exStyle
        pushWord(7);  pushWord(7);  pushWord(166); pushWord(12); // x, y, cx, cy
        pushWord(10);             // ID
        pushWord(0xFFFF); pushWord(0x0082); // class: Static
        pushStringW(L"");         // text (set in WM_INITDIALOG)
        pushWord(0);              // creation data

        // Control 2: Edit box (ID=11)
        align(4);
        pushDword(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL); // style
        pushDword(0);             // exStyle
        pushWord(7);  pushWord(22); pushWord(166); pushWord(14); // x, y, cx, cy
        pushWord(11);             // ID
        pushWord(0xFFFF); pushWord(0x0081); // class: Edit
        pushStringW(L"");         // text
        pushWord(0);              // creation data

        // Control 3: OK button (IDOK=1)
        align(4);
        pushDword(WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON); // style
        pushDword(0);             // exStyle
        pushWord(40); pushWord(46); pushWord(50); pushWord(14); // x, y, cx, cy
        pushWord(IDOK);           // ID
        pushWord(0xFFFF); pushWord(0x0080); // class: Button
        pushStringW(L"OK");       // text
        pushWord(0);              // creation data

        // Control 4: Cancel button (IDCANCEL=2)
        align(4);
        pushDword(WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON); // style
        pushDword(0);             // exStyle
        pushWord(95); pushWord(46); pushWord(50); pushWord(14); // x, y, cx, cy
        pushWord(IDCANCEL);       // ID
        pushWord(0xFFFF); pushWord(0x0080); // class: Button
        pushStringW(L"Cancel");   // text
        pushWord(0);              // creation data

        return buf;
    }

    bool ShowInputDialog(const char* title, const char* prompt, std::string& outText, const std::string& defaultText) {
        InputDialogData data = {};
        data.prompt = prompt;
        strncpy_s(data.buffer, defaultText.c_str(), sizeof(data.buffer) - 1);

        std::vector<BYTE> tmpl = BuildInputDialogTemplate(title);

        INT_PTR result = ::DialogBoxIndirectParamA(
            ::GetModuleHandle(NULL),
            reinterpret_cast<LPCDLGTEMPLATEA>(tmpl.data()),
            NULL, // No owner - works reliably regardless of focus state
            InputDialogProc,
            reinterpret_cast<LPARAM>(&data)
        );

        if (result == IDOK) {
            outText = data.buffer;
            return true;
        }
        return false;
    }

    // --- Win32 Edit Teleport Dialog ---

    static INT_PTR CALLBACK EditTeleportDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_INITDIALOG: {
            EditTeleportResult* data = reinterpret_cast<EditTeleportResult*>(lParam);
            ::SetWindowLongPtrA(hDlg, GWLP_USERDATA, (LONG_PTR)data);
            ::SetDlgItemTextA(hDlg, 100, data->name);
            char buf[64];
            snprintf(buf, sizeof(buf), "%.6f", data->coords[0]); ::SetDlgItemTextA(hDlg, 101, buf);
            snprintf(buf, sizeof(buf), "%.6f", data->coords[1]); ::SetDlgItemTextA(hDlg, 102, buf);
            snprintf(buf, sizeof(buf), "%.6f", data->coords[2]); ::SetDlgItemTextA(hDlg, 103, buf);
            snprintf(buf, sizeof(buf), "%d", data->mapId);       ::SetDlgItemTextA(hDlg, 104, buf);
            ::SetFocus(::GetDlgItem(hDlg, 100));
            ::SendDlgItemMessageA(hDlg, 100, EM_SETSEL, 0, -1);
            return FALSE;
        }
        case WM_COMMAND: {
            EditTeleportResult* data = reinterpret_cast<EditTeleportResult*>(::GetWindowLongPtrA(hDlg, GWLP_USERDATA));
            if (LOWORD(wParam) == 105) { // Set Current Position button
                data->setCurrentPos = true;
                ::EndDialog(hDlg, 105); // Special return code
                return TRUE;
            }
            if (LOWORD(wParam) == IDOK) {
                ::GetDlgItemTextA(hDlg, 100, data->name, sizeof(data->name));
                char buf[64];
                ::GetDlgItemTextA(hDlg, 101, buf, sizeof(buf)); data->coords[0] = (float)atof(buf);
                ::GetDlgItemTextA(hDlg, 102, buf, sizeof(buf)); data->coords[1] = (float)atof(buf);
                ::GetDlgItemTextA(hDlg, 103, buf, sizeof(buf)); data->coords[2] = (float)atof(buf);
                ::GetDlgItemTextA(hDlg, 104, buf, sizeof(buf)); data->mapId = atoi(buf);
                data->setCurrentPos = false;
                ::EndDialog(hDlg, IDOK);
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) {
                ::EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
        }
        case WM_CLOSE:
            ::EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        return FALSE;
    }

    static std::vector<BYTE> BuildEditTeleportDialogTemplate(const char* title) {
        std::vector<BYTE> buf;
        buf.reserve(2048);

        auto align = [&](int boundary) {
            while (buf.size() % boundary != 0) buf.push_back(0);
        };
        auto pushWord = [&](WORD w) {
            buf.push_back((BYTE)(w & 0xFF));
            buf.push_back((BYTE)((w >> 8) & 0xFF));
        };
        auto pushDword = [&](DWORD dw) {
            pushWord((WORD)(dw & 0xFFFF));
            pushWord((WORD)((dw >> 16) & 0xFFFF));
        };
        auto pushStringW = [&](const wchar_t* s) {
            for (; *s; s++) pushWord((WORD)*s);
            pushWord(0);
        };
        auto pushStringA = [&](const char* s) {
            while (*s) { pushWord((WORD)(unsigned char)*s); s++; }
            pushWord(0);
        };
        auto pushControl = [&](DWORD style, WORD x, WORD y, WORD cx, WORD cy, WORD id, WORD classId, const wchar_t* text) {
            align(4);
            pushDword(style);
            pushDword(0); // exStyle
            pushWord(x); pushWord(y); pushWord(cx); pushWord(cy);
            pushWord(id);
            pushWord(0xFFFF); pushWord(classId);
            pushStringW(text);
            pushWord(0); // creation data
        };

        // Layout constants (dialog units)
        const WORD dlgW = 220, dlgH = 160;
        const WORD lblX = 7, editX = 50, editW = 163, editH = 14, lblH = 10;
        WORD curY = 7;

        // DLGTEMPLATE - 13 controls
        DWORD style = DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_SETFONT;
        pushDword(style);
        pushDword(0);
        pushWord(13);  // number of controls
        pushWord(0); pushWord(0);
        pushWord(dlgW); pushWord(dlgH);
        pushWord(0); // menu
        pushWord(0); // class
        pushStringA(title);
        pushWord(9);
        pushStringW(L"Segoe UI");

        // Name label + edit
        pushControl(WS_CHILD | WS_VISIBLE | SS_LEFT, lblX, curY + 2, 40, lblH, (WORD)-1, 0x0082, L"Name:");
        pushControl(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, editX, curY, editW, editH, 100, 0x0081, L"");
        curY += 18;

        // X label + edit
        pushControl(WS_CHILD | WS_VISIBLE | SS_LEFT, lblX, curY + 2, 40, lblH, (WORD)-1, 0x0082, L"X:");
        pushControl(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, editX, curY, editW, editH, 101, 0x0081, L"");
        curY += 18;

        // Y label + edit
        pushControl(WS_CHILD | WS_VISIBLE | SS_LEFT, lblX, curY + 2, 40, lblH, (WORD)-1, 0x0082, L"Y:");
        pushControl(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, editX, curY, editW, editH, 102, 0x0081, L"");
        curY += 18;

        // Z label + edit
        pushControl(WS_CHILD | WS_VISIBLE | SS_LEFT, lblX, curY + 2, 40, lblH, (WORD)-1, 0x0082, L"Z:");
        pushControl(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, editX, curY, editW, editH, 103, 0x0081, L"");
        curY += 18;

        // Map ID label + edit
        pushControl(WS_CHILD | WS_VISIBLE | SS_LEFT, lblX, curY + 2, 40, lblH, (WORD)-1, 0x0082, L"Map ID:");
        pushControl(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, editX, curY, editW, editH, 104, 0x0081, L"");
        curY += 20;

        // Buttons: Set Current Position, Save, Cancel
        pushControl(WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 7, curY, 75, 14, 105, 0x0080, L"Set Current Pos");
        pushControl(WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 110, curY, 50, 14, IDOK, 0x0080, L"Save");
        pushControl(WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 163, curY, 50, 14, IDCANCEL, 0x0080, L"Cancel");

        return buf;
    }

    bool ShowEditTeleportDialog(const char* title, EditTeleportResult& data) {
        std::vector<BYTE> tmpl = BuildEditTeleportDialogTemplate(title);

        INT_PTR result = ::DialogBoxIndirectParamA(
            ::GetModuleHandle(NULL),
            reinterpret_cast<LPCDLGTEMPLATEA>(tmpl.data()),
            NULL,
            EditTeleportDialogProc,
            reinterpret_cast<LPARAM>(&data)
        );

        return (result == IDOK || result == 105);
    }

}
