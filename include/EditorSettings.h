#pragma once
#include <string>

class EditorSettings {
public:
    std::string wimeDIRECTORY;
    std::string lastOpenedFile;
    bool autoSave;
    bool showDebugInfo;
    int windowWidth;
    int windowHeight;
    bool windowMaximized;

    EditorSettings();
}; 