#pragma once
#include <string>
#include <vector>
#include <functional>
#include "Game.h"
#include "ResourceIndex.h"

class ResourceBrowserWindow {
public:
    ResourceBrowserWindow();
    ~ResourceBrowserWindow();
    
    void Render();
    void SetGame(const Game* game);
    void ClearGame();
    
    // Resource selection callbacks
    void SetOnResourceSelected(std::function<void(const std::shared_ptr<ResourceItem>&)> callback);
    
private:
    const Game* currentGame;
    bool hasGame;
    std::function<void(const std::shared_ptr<ResourceItem>&)> onResourceSelected;
    
    void RenderResourceTab(const char* tabName, ResourceType type);
    void RenderResourceList(const std::vector<std::shared_ptr<ResourceItem>>& items);
    void RenderNoResourcesMessage();
}; 