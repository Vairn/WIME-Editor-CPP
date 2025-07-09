#include "EditorUI.h"
#include "GameInfoWindow.h"
#include "ResourceBrowserWindow.h"
#include "PropertiesWindow.h"
#include "PreviewWindow.h"
#include "ConsoleWindow.h"
#include "ResourceLoader.h"
#include <imgui.h>
#include <GLFW/glfw3.h>

EditorUI::EditorUI() 
    : gameLoaded(false)
    , showGameInfo(true)
    , showResourceBrowser(true)
    , showProperties(true)
    , showPreview(false)
    , showConsole(true)
    , shouldOpenFile(false) {
    
    // Initialize file filters
    wimeFilters = {
        {"WIME Executables", "*.exe"},
        {"Amiga Executables", "*.prg"},
        {"Apple IIGS Files", "*.sys16"},
        {"All Files", "*"}
    };
}

EditorUI::~EditorUI() = default;

void EditorUI::Initialize() {
    // Create window components
    gameInfoWindow = std::make_unique<GameInfoWindow>();
    resourceBrowserWindow = std::make_unique<ResourceBrowserWindow>();
    propertiesWindow = std::make_unique<PropertiesWindow>();
    previewWindow = std::make_unique<PreviewWindow>();
    consoleWindow = std::make_unique<ConsoleWindow>();
    
    // Set up console command callback
    consoleWindow->SetCommandCallback([this](const std::string& command) {
        // Handle console commands
        consoleWindow->AddMessage("Command: " + command);
    });
    
    // Set up resource selection callback
    resourceBrowserWindow->SetOnResourceSelected([this](const std::shared_ptr<ResourceItem>& resource) {
        propertiesWindow->SetSelectedResource(resource);
        previewWindow->SetResource(resource, currentGame ? currentGame->FilePath : "");
        consoleWindow->AddMessage("Selected resource: " + resource->name);
    });
    
    // Set up ResourceLoader debug callback
    ResourceLoader::SetDebugCallback([this](const std::string& message) {
        consoleWindow->AddMessage("[ResourceLoader] " + message);
    });
    
    // Set up Game debug callback
    Game::SetDebugCallback([this](const std::string& message) {
        consoleWindow->AddMessage("[Game] " + message);
    });
}

void EditorUI::Render() {
    RenderMainMenuBar();
    RenderDockSpace();
    
    // Render windows based on visibility
    if (showGameInfo) {
        gameInfoWindow->Render();
    }
    if (showResourceBrowser) {
        resourceBrowserWindow->Render();
    }
    if (showProperties) {
        propertiesWindow->Render();
    }
    if (showPreview) {
        previewWindow->Render();
    }
    if (showConsole) {
        consoleWindow->Render();
    }
}

void EditorUI::Shutdown() {
    gameInfoWindow.reset();
    resourceBrowserWindow.reset();
    propertiesWindow.reset();
    previewWindow.reset();
    consoleWindow.reset();
}

void EditorUI::SetGame(std::unique_ptr<Game> game) {
    currentGame = std::move(game);
    gameLoaded = currentGame != nullptr;
    
    consoleWindow->AddMessage("SetGame called - gameLoaded: " + std::string(gameLoaded ? "true" : "false"));
    
    // Update window components
    if (gameLoaded) {
        gameInfoWindow->SetGame(currentGame.get());
        resourceBrowserWindow->SetGame(currentGame.get());
        propertiesWindow->SetGameFilePath(currentGame->FilePath);
        previewWindow->SetResource(nullptr, currentGame->FilePath);
        consoleWindow->AddMessage("Game loaded: " + currentGame->Name);
        consoleWindow->AddMessage("Game file: " + currentGame->FilePath);
        consoleWindow->AddMessage("Game format: " + std::to_string(static_cast<int>(currentGame->format)));
        
        if (currentGame->resource) {
            consoleWindow->AddMessage("Resource index found with " + std::to_string(currentGame->resource->items.size()) + " items");
        } else {
            consoleWindow->AddMessage("No resource index found");
        }
    } else {
        gameInfoWindow->ClearGame();
        resourceBrowserWindow->ClearGame();
        propertiesWindow->ClearSelection();
        previewWindow->SetResource(nullptr, "");
        consoleWindow->AddMessage("Game unloaded");
    }
}

void EditorUI::ClearGame() {
    SetGame(nullptr);
}

bool EditorUI::HasGame() const {
    return gameLoaded;
}

void EditorUI::SetSettings(const EditorSettings& newSettings) {
    settings = newSettings;
}

EditorSettings& EditorUI::GetSettings() {
    return settings;
}

const std::vector<FileDialog::FileFilter>& EditorUI::GetWimeFilters() const {
    return wimeFilters;
}

void EditorUI::ShowGameInfo(bool show) {
    showGameInfo = show;
}

void EditorUI::ShowResourceBrowser(bool show) {
    showResourceBrowser = show;
}

void EditorUI::ShowProperties(bool show) {
    showProperties = show;
}

void EditorUI::ShowPreview(bool show) {
    showPreview = show;
}

void EditorUI::ShowConsole(bool show) {
    showConsole = show;
}

void EditorUI::OnFileOpen(const std::string& filePath) {
    settings.lastOpenedFile = filePath;
    consoleWindow->AddMessage("Opening file: " + filePath);
}

void EditorUI::OnExit() {
    consoleWindow->AddMessage("Exiting application...");
}

void EditorUI::OnAbout() {
    consoleWindow->AddMessage("WIME Editor C++ v0.1.0");
}

void EditorUI::RenderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Game", "Ctrl+O")) {
                shouldOpenFile = true;
            }
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                OnExit();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Game Info", nullptr, &showGameInfo);
            ImGui::MenuItem("Resource Browser", nullptr, &showResourceBrowser);
            ImGui::MenuItem("Properties", nullptr, &showProperties);
            ImGui::MenuItem("Preview", nullptr, &showPreview);
            ImGui::MenuItem("Console", nullptr, &showConsole);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                OnAbout();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void EditorUI::RenderDockSpace() {
    SetupDockSpace();
    
    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpace(ImGui::GetID("MyDockSpace"));
    }
    
    ImGui::End(); // End DockSpace
}

void EditorUI::SetupDockSpace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(2);
} 