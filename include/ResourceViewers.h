#pragma once

#include <memory>
#include <string>
#include <vector>
#include "ResourceIndex.h"
#include <imgui.h>

// Forward declarations
class ResourceItem;

// Base class for resource viewers
class ResourceViewer {
public:
    virtual ~ResourceViewer() = default;
    virtual void RenderProperties() = 0;  // For Properties window
    virtual void RenderPreview() = 0;     // For Preview window
    virtual void SetResource(const std::shared_ptr<ResourceItem>& resource) = 0;
    virtual void SetGameFilePath(const std::string& filePath) = 0;
    virtual void ClearCache() = 0;
};

// String resource viewer (CSTR)
class StringResourceViewer : public ResourceViewer {
private:
    std::shared_ptr<ResourceItem> resource;
    std::string gameFilePath;
    std::string cachedData;
    bool dataLoaded = false;

    std::string LoadStringData();

public:
    void RenderProperties() override;
    void RenderPreview() override;
    void SetResource(const std::shared_ptr<ResourceItem>& resource) override;
    void SetGameFilePath(const std::string& filePath) override;
    void ClearCache() override;
};

// Map resource viewer (MMAP)
class MapResourceViewer : public ResourceViewer {
private:
    std::shared_ptr<ResourceItem> resource;
    std::string gameFilePath;
    std::vector<uint8_t> cachedDecompressedData;
    bool dataLoaded = false;
    
    // Map properties
    uint32_t width = 2560;
    uint32_t height = 1584;
    uint16_t planes = 1;
    uint32_t mapGridWidth = 160;
    uint32_t mapGridHeight = 99;
    static constexpr uint32_t TILE_SIZE = 16;
    
    // Cached rendered map image
    static std::vector<ImU32> cachedMapImage;
    static std::vector<uint8_t> lastMapData;
    static std::vector<uint8_t> lastTileData;
    static bool imageValid;
    
    std::vector<uint8_t> DecompressMapData();
    void RenderMapGrid();
    void RenderMapProperties();
    void RenderMapWithTiles(const std::vector<uint8_t>& mapData, const std::vector<uint8_t>& tileData);
    void RenderMapToImage(const std::vector<uint8_t>& mapData, const std::vector<uint8_t>& tileData);

public:
    void RenderProperties() override;
    void RenderPreview() override;
    void SetResource(const std::shared_ptr<ResourceItem>& resource) override;
    void SetGameFilePath(const std::string& filePath) override;
    void ClearCache() override;
};

class CharResourceViewer : public ResourceViewer {
private:
    std::shared_ptr<ResourceItem> resource;
    std::string gameFilePath;
    std::vector<uint8_t> cachedDecompressedData;
    bool dataLoaded = false;
    static constexpr uint32_t TILE_COUNT = 256;
    static constexpr uint32_t TILE_SIZE = 16;
    static constexpr uint32_t TILE_BYTES = 128; // 16x16 pixels, 4bpp = 2 pixels per byte

    // PC VGA palette based on VB code hex colors using ImGui color conversion (swapped R/B)
    static ImU32 GetPaletteColor(uint8_t index) {
        static const char* hexColors[16] = {
            "000000", // 0: Black
            "5586FF", // 1: Blue (from VGATilePalette index 1)
            "306510", // 2: Green (from VGATilePalette index 2)
            "755555", // 3: Brown (from VGATilePalette index 3)
            "EBAA86", // 4: Light orange (from VGATilePalette index 4)
            "00FFFF", // 5: Cyan (from VGATilePalette index 5)
            "204110", // 6: Dark green (from VGATilePalette index 6)
            "659655", // 7: Light green (from VGATilePalette index 7)
            "868686", // 8: Gray (from VGATilePalette index 8)
            "86BAFF", // 9: Light blue (from VGATilePalette index 9)
            "CB0041", // 10: Red (from VGATilePalette index 10)
            "FFFFFF", // 11: White (from VGATilePalette index 11)
            "DB75CB", // 12: Pink (from VGATilePalette index 12)
            "65BA00", // 13: Bright green (from VGATilePalette index 13)
            "EBEBBA", // 14: Light yellow (from VGATilePalette index 14)
            "FFFFDB"  // 15: Very light yellow (from VGATilePalette index 15)
        };
        
        if (index < 16) {
            uint32_t r, g, b;
            sscanf_s(hexColors[index], "%02x%02x%02x", &r, &g, &b);
            // Swap red and green for ImGui
            return IM_COL32(g, r, b, 255);
        }
        return IM_COL32(0, 0, 0, 255); // Black fallback
    }

public:
    void SetResource(const std::shared_ptr<ResourceItem>& resource) override;
    void SetGameFilePath(const std::string& filePath) override;
    void ClearCache() override;
    void RenderProperties() override;
    void RenderPreview() override;

    // Static method to get tile data for use by other viewers
    static std::vector<uint8_t> GetTileData(const std::string& sourceFile, uint32_t offset);
    static std::vector<uint8_t> DecodeTile(const std::vector<uint8_t>& tileData, size_t tileIndex);
    static ImU32 GetTileColor(uint8_t pixelValue);

private:
    std::vector<uint8_t> DecompressTileData();
};

// Generic binary resource viewer (for other types)
class BinaryResourceViewer : public ResourceViewer {
private:
    std::shared_ptr<ResourceItem> resource;
    std::string gameFilePath;
    std::vector<uint8_t> cachedData;
    bool dataLoaded = false;

    std::vector<uint8_t> LoadBinaryData();
    void RenderHexDump(const std::vector<uint8_t>& data, size_t maxBytes = 64);

public:
    void RenderProperties() override;
    void RenderPreview() override;
    void SetResource(const std::shared_ptr<ResourceItem>& resource) override;
    void SetGameFilePath(const std::string& filePath) override;
    void ClearCache() override;
};

// Factory function to create appropriate viewer
std::unique_ptr<ResourceViewer> CreateResourceViewer(ResourceType type); 