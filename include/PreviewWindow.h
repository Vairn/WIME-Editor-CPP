#pragma once

#include <memory>
#include <string>
#include "ResourceIndex.h"
#include "ResourceViewers.h"

class PreviewWindow {
public:
    PreviewWindow();
    ~PreviewWindow();

    void SetResource(const std::shared_ptr<ResourceItem>& resource, const std::string& gameFilePath);
    void Render();
    bool IsOpen() const { return isOpen; }
    void Close() { isOpen = false; }
    std::string GetTitle() const;

private:
    std::shared_ptr<ResourceItem> resource;
    std::string gameFilePath;
    std::unique_ptr<ResourceViewer> viewer;
    bool isOpen = true;

}; 