#include "PropertiesWindow.h"
#include <imgui.h>
#include <iostream>
#include <filesystem>
#include <algorithm>

PropertiesWindow::PropertiesWindow() 
    : hasSelection(false) {
}

PropertiesWindow::~PropertiesWindow() = default;

void PropertiesWindow::Render() {
    if (ImGui::Begin("Properties", nullptr)) {
        if (hasSelection && selectedResource) {
            RenderResourceProperties();
        } else {
            RenderNoSelectionMessage();
        }
    }
    ImGui::End();
}

void PropertiesWindow::SetSelectedResource(const std::shared_ptr<ResourceItem>& resource) {
    selectedResource = resource;
    hasSelection = (resource != nullptr);
    UpdateViewer();
}

void PropertiesWindow::SetGameFilePath(const std::string& filePath) {
    gameFilePath = filePath;
    // Update viewer with new game file path
    if (currentViewer) {
        currentViewer->SetGameFilePath(filePath);
    }
}

void PropertiesWindow::ClearSelection() {
    selectedResource = nullptr;
    hasSelection = false;
}

void PropertiesWindow::RenderResourceProperties() {
    ImGui::Text("Resource Properties");
    ImGui::Separator();
    
    RenderResourceDetails();
}

void PropertiesWindow::UpdateViewer() {
    if (!selectedResource) {
        currentViewer.reset();
        return;
    }
    
    // Create appropriate viewer for the resource type
    currentViewer = CreateResourceViewer(selectedResource->type);
    if (currentViewer) {
        currentViewer->SetResource(selectedResource);
        currentViewer->SetGameFilePath(gameFilePath);
    }
}

void PropertiesWindow::RenderResourceDetails() {
    if (currentViewer) {
        currentViewer->RenderProperties();
    } else {
        ImGui::Text("No resource selected");
    }
}

void PropertiesWindow::RenderNoSelectionMessage() {
    ImGui::Text("Resource Properties");
    ImGui::Text("Select a resource to view its properties");
} 