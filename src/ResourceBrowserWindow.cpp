#include "ResourceBrowserWindow.h"
#include <imgui.h>
#include <filesystem>

ResourceBrowserWindow::ResourceBrowserWindow() 
    : currentGame(nullptr)
    , hasGame(false) {
}

ResourceBrowserWindow::~ResourceBrowserWindow() = default;

void ResourceBrowserWindow::Render() {
    if (ImGui::Begin("Resource Browser", nullptr)) {
        if (hasGame && currentGame && currentGame->resource) {
            // Resource tabs
            if (ImGui::BeginTabBar("ResourceTabs")) {
                RenderResourceTab("Characters", ResourceType::CHAR);
                RenderResourceTab("Strings", ResourceType::CSTR);
                RenderResourceTab("Fonts", ResourceType::FONT);
                RenderResourceTab("Images", ResourceType::IMAG);
                RenderResourceTab("Maps", ResourceType::MMAP);
                ImGui::EndTabBar();
            }
        } else {
            RenderNoResourcesMessage();
        }
    }
    ImGui::End();
}

void ResourceBrowserWindow::SetGame(const Game* game) {
    currentGame = game;
    hasGame = (game != nullptr);
}

void ResourceBrowserWindow::ClearGame() {
    currentGame = nullptr;
    hasGame = false;
}

void ResourceBrowserWindow::SetOnResourceSelected(std::function<void(const std::shared_ptr<ResourceItem>&)> callback) {
    onResourceSelected = callback;
}

void ResourceBrowserWindow::RenderResourceTab(const char* tabName, ResourceType type) {
    if (ImGui::BeginTabItem(tabName)) {
        ImGui::Text("%s Resources", tabName);
        
        auto items = currentGame->resource->GetItemsByType(type);
        if (!items.empty()) {
            RenderResourceList(items);
        } else {
            ImGui::Text("No %s resources found", tabName);
        }
        
        ImGui::EndTabItem();
    }
}

void ResourceBrowserWindow::RenderResourceList(const std::vector<std::shared_ptr<ResourceItem>>& items) {
    for (const auto& item : items) {
        // Create display name with source file info
        std::string displayName = item->name;
        if (!item->sourceFile.empty()) {
            // Extract just the filename from the full path
            std::filesystem::path sourcePath(item->sourceFile);
            std::string filename = sourcePath.filename().string();
            displayName += " [" + filename + "]";
        }
        
        if (ImGui::Selectable(displayName.c_str())) {
            if (onResourceSelected) {
                onResourceSelected(item);
            }
        }
        
        // Show tooltip with additional info
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Name: %s", item->name.c_str());
            ImGui::Text("Source: %s", item->sourceFile.c_str());
            ImGui::Text("Offset: 0x%08X", item->offset);
            ImGui::Text("Size: %u bytes", item->size);
            ImGui::EndTooltip();
        }
    }
}

void ResourceBrowserWindow::RenderNoResourcesMessage() {
    ImGui::Text("No resources loaded");
    if (!hasGame) {
        ImGui::Text("Load a game to view resources");
    } else if (!currentGame->resource) {
        ImGui::Text("No resource data available");
    }
} 