#pragma once
#include <string>
#include <unordered_map>
#include "ResourceIndex.h"
#include "BinaryFile.h"
#include "ResourceViewers.h"

class PropertiesWindow {
public:
    PropertiesWindow();
    ~PropertiesWindow();
    
    const char* GetResourceTypeString(ResourceType type);
    //const char* GetResourceFormatString(ResourceFormat format);
    
    void Render();
    void SetSelectedResource(const std::shared_ptr<ResourceItem>& resource);
    void SetGameFilePath(const std::string& filePath);
    void ClearSelection();
    
    
private:
    std::shared_ptr<ResourceItem> selectedResource;
    bool hasSelection;
    std::string gameFilePath;
    
    // Current resource viewer
    std::unique_ptr<ResourceViewer> currentViewer;
    
    void UpdateViewer();
    
    void RenderResourceProperties();
    void RenderNoSelectionMessage();
    void RenderResourceDetails();
}; 