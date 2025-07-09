#pragma once
#include <string>
#include <vector>
#include <memory>

// Resource types
enum class ResourceType {
    CHAR,   // Characters
    CSTR,   // Strings
    FONT,   // Fonts
    FRML,   // Forms/UI
    IMAG,   // Images
    MMAP,   // Maps
    ARCHIVE // Archive
};

struct ResourceItem {
    std::string name;
    uint32_t offset;
    uint32_t size;
    ResourceType type;
    std::string sourceFile;  // The .res file this resource comes from
    
    ResourceItem(const std::string& n = "", uint32_t off = 0, uint32_t sz = 0, ResourceType t = ResourceType::CHAR, const std::string& file = "")
        : name(n), offset(off), size(sz), type(t), sourceFile(file) {}
};

class ResourceIndex {
public:
    std::string ID;
    std::vector<std::shared_ptr<ResourceItem>> items;
    
    ResourceIndex() = default;
    ResourceIndex(const std::string& id) : ID(id) {}
    
    void AddItem(const std::string& name, uint32_t offset, uint32_t size, ResourceType type, const std::string& sourceFile = "") {
        items.emplace_back(std::make_shared<ResourceItem>(name, offset, size, type, sourceFile));
    }
    
    std::vector<std::shared_ptr<ResourceItem>> GetItemsByType(ResourceType type) const {
        std::vector<std::shared_ptr<ResourceItem>> result;
        for (const auto& item : items) {
            if (item->type == type) {
                result.push_back(item);
            }
        }
        return result;
    }
    
    size_t GetItemCount(ResourceType type) const {
        size_t count = 0;
        for (const auto& item : items) {
            if (item->type == type) count++;
        }
        return count;
    }
}; 