void Hack::initializeOffsets()
{
    m_xOffsets = { BYTE1, BYTE2, BYTE3, BYTE4, 0x120 };
    m_yOffsets = { BYTE1, BYTE2, BYTE3, BYTE4, 0x124 };  // 修正: 原本是 0x128
    m_zOffsets = { BYTE1, BYTE2, BYTE3, BYTE4, 0x128 };  // 修正: 原本是 0x124
    m_zHeight1Offsets = { BYTE1, BYTE2, BYTE3, BYTE4, 0x118 };
    m_zHeight2Offsets = { BYTE1, BYTE2, BYTE3, BYTE4, 0x114 };
    m_gravityOffsets = { BYTE1, BYTE2, BYTE3, 0x1FC };
    m_speedOffsets = { BYTE1, BYTE2, BYTE3, 0x220 };
    m_wallClimbOffsets = { BYTE1, BYTE2, BYTE3, 0x204 };
}