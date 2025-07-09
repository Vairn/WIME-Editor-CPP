#include "GameInfoWindow.h"
#include <imgui.h>

GameInfoWindow::GameInfoWindow() 
    : currentGame(nullptr)
    , hasGame(false) {
}

GameInfoWindow::~GameInfoWindow() = default;

void GameInfoWindow::Render() {
    if (ImGui::Begin("Game Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (hasGame && currentGame) {
            RenderGameInfo();
        } else {
            RenderNoGameMessage();
        }
    }
    ImGui::End();
}

void GameInfoWindow::SetGame(const Game* game) {
    currentGame = game;
    hasGame = (game != nullptr);
}

void GameInfoWindow::ClearGame() {
    currentGame = nullptr;
    hasGame = false;
}

void GameInfoWindow::RenderGameInfo() {
    ImGui::Text("Game: %s", currentGame->Name.c_str());
    ImGui::Text("File: %s", currentGame->FilePath.c_str());
    ImGui::Text("Format: %s", GetFormatString(currentGame->format));
    ImGui::Text("Loaded: %s", currentGame->IsLoaded ? "Yes" : "No");
    
    if (currentGame->resource) {
        ImGui::Separator();
        ImGui::Text("Resources:");
        ImGui::Text("  Characters: %zu", currentGame->resource->GetItemCount(ResourceType::CHAR));
        ImGui::Text("  Strings: %zu", currentGame->resource->GetItemCount(ResourceType::CSTR));
        ImGui::Text("  Fonts: %zu", currentGame->resource->GetItemCount(ResourceType::FONT));
        ImGui::Text("  Images: %zu", currentGame->resource->GetItemCount(ResourceType::IMAG));
        ImGui::Text("  Maps: %zu", currentGame->resource->GetItemCount(ResourceType::MMAP));
    }
}

void GameInfoWindow::RenderNoGameMessage() {
    ImGui::Text("Status: No game loaded");
    ImGui::Text("Use File -> Open Game to load a WIME game");
}

const char* GameInfoWindow::GetFormatString(GameFormat format) const {
    switch (format) {
        case GameFormat::PC: return "PC";
        case GameFormat::Amiga: return "Amiga";
        case GameFormat::AppleIIGS: return "Apple IIGS";
        case GameFormat::AtariST: return "Atari ST";
        default: return "Unknown";
    }
} 