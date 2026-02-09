# 快速修改指南

## 已完成的所有修改

### ✅ 1. 應用程式名稱更改
**從**: KX Trainer  
**到**: Jack's GW2 External

**修改的文件**:
- `constants.h` - APP_NAME 常量
- `hack_gui.cpp` - 視窗標題和 Info 區段

---

### ✅ 2. 圖標按鈕替代文字按鈕
使用 Unicode 符號替代文字，節省空間且更美觀：

| 按鈕 | 圖標 | Unicode | Tooltip |
|------|------|---------|---------|
| Add Group | ⊕ | U+002B | "Add Group" |
| Rename Group | ✎ | U+270E | "Rename Group" |
| Delete Group | ✖ | U+2716 | "Delete Group" |
| Import JSON | 📥 | U+1F4E5 | "Import JSON" |

**修改的文件**:
- `hack_gui.cpp` - RenderTeleportsTab() 函數

---

### ✅ 3. Import JSON 文件選擇器
點擊 Import JSON 按鈕現在會：
1. 彈出 Windows 文件選擇對話框
2. 僅顯示 `.json` 文件
3. 選擇文件後自動讀取並導入
4. 顯示成功或失敗訊息

**新增的文件**:
- `ui_helpers.h` - 工具函數聲明
- `ui_helpers.cpp` - 文件對話框實現

**修改的文件**:
- `hack_gui.cpp` - Import 按鈕功能

---

### ✅ 4. Delete Teleport Bug 修復

**問題**: 
切換群組後點擊刪除按鈕可能無反應或刪除錯誤的項目

**原因**: 
只儲存了 teleport index，沒有儲存 group index

**解決方案**:
1. 新增 `m_deleteTeleportGroupIndex` 成員變數
2. 點擊刪除時同時儲存群組和傳送點索引
3. 確認視窗使用儲存的索引
4. 按鈕 ID 更改為包含群組索引：`"X##tp_{groupIdx}_{tpIdx}"`

**修改的文件**:
- `hack_gui.h` - 新增成員變數
- `hack_gui.cpp` - 刪除邏輯修復

---

## 如何使用新功能

### 圖標按鈕
- **懸停**在圖標上會顯示功能說明
- 點擊方式與原文字按鈕相同

### Import JSON
1. 點擊 📥 按鈕
2. 瀏覽並選擇 `.json` 文件
3. 點擊「開啟」
4. 查看狀態訊息確認導入成功

### 刪除傳送點
- 現在可以安全地在不同群組之間切換
- 刪除確認視窗會正確顯示要刪除的項目
- 確認後會刪除正確的傳送點

---

## 圖標資源（可選升級）

### 當前狀態
使用 Unicode 符號（✅ 可用，但字體依賴）

### 升級到真實圖標
查看 `ICON_RESOURCES.md` 了解：
- 如何準備圖標文件
- 圖標尺寸建議
- 免費圖標資源網站
- 實現代碼範例

---

## 建置狀態
✅ **編譯成功** - 所有修改已測試並通過編譯

## 相容性
- ✅ C++17 標準
- ✅ Windows API
- ✅ ImGui
- ✅ 現有功能完全保留

---

## 問題回報
如果遇到問題，請檢查：
1. 文件是否正確包含
2. 編譯器設定是否為 C++17
3. ImGui 字體是否支援 Unicode
