#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Game.h"
#include "EditorSettings.h"
#include "FileDialog.h"

// Forward declarations
class GameInfoWindow;
class ResourceBrowserWindow;
class PropertiesWindow;
class PreviewWindow;
class ConsoleWindow;

class EditorUI {
public:
    EditorUI();
    ~EditorUI();
    
    // Main UI management
    void Initialize();
    void Render();
    void Shutdown();
    
    // Game state management
    void SetGame(std::unique_ptr<Game> game);
    void ClearGame();
    bool HasGame() const;
    
    // Settings
    void SetSettings(const EditorSettings& settings);
    EditorSettings& GetSettings();
    
    // File dialog filters
    const std::vector<FileDialog::FileFilter>& GetWimeFilters() const;
    
    // Window visibility control
    void ShowGameInfo(bool show = true);
    void ShowResourceBrowser(bool show = true);
    void ShowProperties(bool show = true);
    void ShowPreview(bool show = true);
    void ShowConsole(bool show = true);
    
    // Event callbacks
    void OnFileOpen(const std::string& filePath);
    void OnExit();
    void OnAbout();
    
    // Menu handling
    bool ShouldOpenFile() const { return shouldOpenFile; }
    void ClearOpenFileFlag() { shouldOpenFile = false; }
    
private:
    // Window components
    std::unique_ptr<GameInfoWindow> gameInfoWindow;
    std::unique_ptr<ResourceBrowserWindow> resourceBrowserWindow;
    std::unique_ptr<PropertiesWindow> propertiesWindow;
    std::unique_ptr<PreviewWindow> previewWindow;
    std::unique_ptr<ConsoleWindow> consoleWindow;
    
    // State
    std::unique_ptr<Game> currentGame;
    EditorSettings settings;
    bool gameLoaded;
    
    // File dialog filters
    std::vector<FileDialog::FileFilter> wimeFilters;
    
    // Window visibility
    bool showGameInfo;
    bool showResourceBrowser;
    bool showProperties;
    bool showPreview;
    bool showConsole;
    
    // Menu state
    bool shouldOpenFile;
    
    // Private methods
    void RenderMainMenuBar();
    void RenderDockSpace();
    void SetupDockSpace();
}; 