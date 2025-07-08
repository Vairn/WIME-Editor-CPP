#include "EditorSettings.h"

// Basic implementation - will be expanded with file I/O later
EditorSettings::EditorSettings() 
    : wimeDIRECTORY(""), lastOpenedFile(""), autoSave(true), 
      showDebugInfo(false), windowWidth(1280), windowHeight(720), 
      windowMaximized(true) {
} 