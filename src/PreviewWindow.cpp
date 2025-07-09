#include "PreviewWindow.h"
#include "BinaryFile.h"
#include <imgui.h>
#include <sstream>

PreviewWindow::PreviewWindow() {
    // Default constructor - no resource set initially
}

void PreviewWindow::SetResource(const std::shared_ptr<ResourceItem>& resource, const std::string& gameFilePath) {
    this->resource = resource;
    this->gameFilePath = gameFilePath;
    
    if (resource) {
        viewer = CreateResourceViewer(resource->type);
        if (viewer) {
            viewer->SetResource(resource);
            viewer->SetGameFilePath(gameFilePath);
        }
    } else {
        viewer.reset();
    }
}

PreviewWindow::~PreviewWindow() = default;

std::string PreviewWindow::GetTitle() const {
    if (!resource) return "Preview";
    std::ostringstream oss;
    oss << "Preview: " << resource->name;
    return oss.str();
}



void PreviewWindow::Render() {
    if (!isOpen) return;
    std::string title = GetTitle();
    title += "###preview";
    if (ImGui::Begin(title.c_str(), &isOpen)) {
        if (!resource) {
            ImGui::Text("No resource selected");
            ImGui::End();
            return;
        }
            if (viewer) {
        viewer->RenderPreview();
    } else {
        ImGui::Text("Preview not implemented for this resource type.");
    }
    }
    ImGui::End();
} 