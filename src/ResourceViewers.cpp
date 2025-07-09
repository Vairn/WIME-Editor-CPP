#include "ResourceViewers.h"
#include "BinaryFile.h"
#include <imgui.h>
#include <iostream>
#include <algorithm>
#include <map>

// Forward declaration
const char* GetResourceTypeString(ResourceType type);

// Factory function implementation
std::unique_ptr<ResourceViewer> CreateResourceViewer(ResourceType type) {
    switch (type) {
        case ResourceType::CSTR:
            return std::make_unique<StringResourceViewer>();
        case ResourceType::MMAP:
            return std::make_unique<MapResourceViewer>();
        case ResourceType::CHAR:
            return std::make_unique<CharResourceViewer>();
        default:
            return std::make_unique<BinaryResourceViewer>();
    }
}

// StringResourceViewer implementation
void StringResourceViewer::SetResource(const std::shared_ptr<ResourceItem>& resource) {
    this->resource = resource;
    dataLoaded = false;
}

void StringResourceViewer::SetGameFilePath(const std::string& filePath) {
    gameFilePath = filePath;
    dataLoaded = false;
}

void StringResourceViewer::ClearCache() {
    cachedData.clear();
    dataLoaded = false;
}

std::string StringResourceViewer::LoadStringData() {
    if (dataLoaded) return cachedData;
    
    try {
        if (resource->sourceFile.empty()) {
            return "Error: No source file specified";
        }
        
        BinaryFile file(resource->sourceFile);
        if (!file.IsOpen()) {
            return "Error: Could not open source file";
        }
        
        // Match VB: seek to offset+4, read exactly size bytes, convert 10 to \n
        size_t start = resource->offset + 4;
        if (start >= file.GetLength()) {
            return "Error: Start position past end of file";
        }
        
        file.SetPosition(start);
        std::string data;
        data.reserve(resource->size);
        
        for (uint32_t i = 0; i < resource->size && file.GetPosition() < file.GetLength(); ++i) {
            uint8_t byte = file.ReadByteUnsigned();
            if (byte == 10) {
                data += '\n';
            } else {
                data += static_cast<char>(byte);
            }
        }
        
        cachedData = data;
        dataLoaded = true;
        return data;
        
    } catch (const std::exception& e) {
        return "Error reading string data: " + std::string(e.what());
    }
}

void StringResourceViewer::RenderProperties() {
    if (!resource) {
        ImGui::Text("No resource selected");
        return;
    }
    
    ImGui::Text("String Resource Properties");
    ImGui::Separator();
    
    ImGui::Text("Name: %s", resource->name.c_str());
    ImGui::Text("Type: String (CSTR)");
    ImGui::Text("Offset: 0x%08X", resource->offset);
    ImGui::Text("Size: %u bytes", resource->size);
    ImGui::Separator();
    
    ImGui::Text("String Content:");
    std::string data = LoadStringData();
    if (!data.empty()) {
        ImGui::TextWrapped("%s", data.c_str());
        
        ImGui::Separator();
        ImGui::Text("String Details:");
        ImGui::Text("Length: %zu characters", data.length());
        ImGui::Text("Bytes: %zu bytes", data.length());
        
        // Show character codes for non-printable characters
        if (!data.empty()) {
            ImGui::Separator();
            ImGui::Text("Character Analysis:");
            for (size_t i = 0; i < data.length() && i < 100; ++i) {
                char c = data[i];
                if (c >= 32 && c <= 126) {
                    ImGui::SameLine();
                    ImGui::Text("%c", c);
                } else {
                    ImGui::SameLine();
                    ImGui::Text("[%02X]", static_cast<unsigned char>(c));
                }
            }
            if (data.length() > 100) {
                ImGui::Text("... (truncated)");
            }
        }
    } else {
        ImGui::Text("(Failed to read string data)");
    }
}

void StringResourceViewer::RenderPreview() {
    if (!resource) {
        ImGui::Text("No resource selected");
        return;
    }
    
    ImGui::Text("String Editor");
    ImGui::Separator();
    
    // Load and display string in editable format
    std::string data = LoadStringData();
    if (!data.empty()) {
        ImGui::Text("Edit the string below:");
        ImGui::PushID(this);
        // ImGui InputTextMultiline requires a char buffer, not std::string*
        static char buffer[4096]; // Static buffer for editing
        strncpy(buffer, data.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        if (ImGui::InputTextMultiline("##editstring", buffer, sizeof(buffer), ImVec2(-1, 300))) {
            // Handle string editing here
            // TODO: Implement save functionality
        }
        ImGui::PopID();
        
        ImGui::Separator();
        ImGui::Text("String Statistics:");
        ImGui::Text("Length: %zu characters", data.length());
        ImGui::Text("Lines: %zu", std::count(data.begin(), data.end(), '\n') + 1);
    } else {
        ImGui::Text("(Failed to read string data)");
    }
}

// MapResourceViewer implementation
void MapResourceViewer::SetResource(const std::shared_ptr<ResourceItem>& resource) {
    this->resource = resource;
    dataLoaded = false;
}

void MapResourceViewer::SetGameFilePath(const std::string& filePath) {
    gameFilePath = filePath;
    dataLoaded = false;
}

void MapResourceViewer::ClearCache() {
    cachedDecompressedData.clear();
    dataLoaded = false;
}

std::vector<uint8_t> MapResourceViewer::DecompressMapData() {
    if (dataLoaded) return cachedDecompressedData;
    
    try {
        if (resource->sourceFile.empty()) {
            return {};
        }
        
        BinaryFile file(resource->sourceFile);
        if (!file.IsOpen()) {
            return {};
        }
        
        // MMAP specific: DataStartOffset = offset + 8, chunkSize = size - 18
        uint32_t dataStartOffset = resource->offset + 8;
        uint32_t chunkSize = resource->size - 18;
        
        if (dataStartOffset >= file.GetLength()) {
            return {};
        }
        
        file.SetPosition(dataStartOffset);
        
        // Calculate expected decompressed size (matching VB logic)
        auto CalculateRowSize = [](uint32_t imageWidth) -> uint32_t {
            uint32_t rowSizeInWords = imageWidth / 16;
            if (imageWidth % 16 != 0) rowSizeInWords++;
            return rowSizeInWords * 2;
        };
        uint32_t expectedDecompressedSize = CalculateRowSize(width) * height * planes;
        std::vector<uint8_t> decompressedData(expectedDecompressedSize, 0);
        
        uint32_t readBytes = 0;
        uint32_t count = 0;
        while (readBytes < chunkSize && count < expectedDecompressedSize) {
            int8_t runByte = file.ReadByteSigned();
            readBytes++;
            if (runByte >= 0 && runByte <= 127) {
                for (int i = 0; i <= runByte && count < expectedDecompressedSize && readBytes < chunkSize; i++) {
                    decompressedData[count++] = file.ReadByteUnsigned();
                    readBytes++;
                }
            } else if (runByte >= -127 && runByte <= -1) {
                uint8_t repeatByte = file.ReadByteUnsigned();
                readBytes++;
                for (int i = 0; i <= -runByte && count < expectedDecompressedSize; i++) {
                    decompressedData[count++] = repeatByte;
                }
            }
        }
        // If decompression ended early, the rest of the buffer remains zero (as in VB)
        cachedDecompressedData = decompressedData;
        dataLoaded = true;
        return decompressedData;
        
    } catch (const std::exception& e) {
        std::cerr << "Error decompressing map data: " << e.what() << std::endl;
        return {};
    }
}



void MapResourceViewer::RenderMapProperties() {
    ImGui::Text("Map Properties:");
    ImGui::Text("  Width: %u pixels", width);
    ImGui::Text("  Height: %u pixels", height);
    ImGui::Text("  Planes: %u", planes);
    ImGui::Text("  Grid: %ux%u tiles", mapGridWidth, mapGridHeight);
    ImGui::Text("  Tile Size: 16x16 pixels");
}

void MapResourceViewer::RenderMapGrid() {
    std::vector<uint8_t> mapData = DecompressMapData();
    if (mapData.empty()) {
        ImGui::Text("(Failed to decompress map data)");
        return;
    }
    
    ImGui::Text("Map Data Preview:");
    ImGui::Text("Decompressed size: %zu bytes", mapData.size());
    
    // Show first few rows of the map grid
    const size_t previewRows = 10;
    const size_t previewCols = 20;
    
    ImGui::Text("First %zu rows, %zu columns:", previewRows, previewCols);
    
    for (size_t row = 0; row < previewRows && row < mapGridHeight; row++) {
        std::string rowText;
        for (size_t col = 0; col < previewCols && col < mapGridWidth; col++) {
            size_t index = row * mapGridWidth + col;
            if (index < mapData.size()) {
                char hex[4];
                snprintf(hex, sizeof(hex), "%02X ", mapData[index]);
                rowText += hex;
            }
        }
        ImGui::Text("Row %2zu: %s", row, rowText.c_str());
    }
    
    if (mapGridHeight > previewRows || mapGridWidth > previewCols) {
        ImGui::Text("... (truncated)");
    }
}

void MapResourceViewer::RenderProperties() {
    if (!resource) {
        ImGui::Text("No resource selected");
        return;
    }
    
    ImGui::Text("Map Resource Properties");
    ImGui::Separator();
    
    ImGui::Text("Name: %s", resource->name.c_str());
    ImGui::Text("Type: Map (MMAP)");
    ImGui::Text("Offset: 0x%08X", resource->offset);
    ImGui::Text("Size: %u bytes", resource->size);
    ImGui::Separator();
    
    RenderMapProperties();
    ImGui::Separator();
    
    RenderMapGrid();
}

void MapResourceViewer::RenderPreview() {
    if (!resource) {
        ImGui::Text("No resource selected");
        return;
    }
    
    ImGui::Text("Map Viewer");
    ImGui::Separator();
    
    std::vector<uint8_t> mapData = DecompressMapData();
    if (mapData.empty()) {
        ImGui::Text("(Failed to decompress map data)");
        return;
    }
    
    ImGui::Text("Grid: %ux%u tiles", mapGridWidth, mapGridHeight);
    ImGui::Separator();
    
    // Load tile data from CHAR resource
    std::vector<uint8_t> tileData = CharResourceViewer::GetTileData(resource->sourceFile, 0xAB7F);
    
    if (!tileData.empty()) {
        RenderMapWithTiles(mapData, tileData);
    } else {
        // Fallback to colored squares if no tile data
        const int tileScale = 2;
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        auto getColor = [](uint8_t value) -> ImU32 {
            return IM_COL32(value, value, value, 255);
        };
        
        for (uint32_t row = 0; row < mapGridHeight; ++row) {
            for (uint32_t col = 0; col < mapGridWidth; ++col) {
                size_t index = row * mapGridWidth + col;
                if (index < mapData.size()) {
                    ImU32 color = getColor(mapData[index]);
                    ImVec2 p0 = ImVec2(canvasPos.x + col * tileScale, canvasPos.y + row * tileScale);
                    ImVec2 p1 = ImVec2(p0.x + tileScale, p0.y + tileScale);
                    drawList->AddRectFilled(p0, p1, color);
                }
            }
        }
        ImGui::Dummy(ImVec2(mapGridWidth * tileScale, mapGridHeight * tileScale));
    }
    
    ImGui::Separator();
    ImGui::Text("Total tiles: %u", mapGridWidth * mapGridHeight);
    ImGui::Text("Tile size: 16x16 pixels (game)");
    ImGui::Text("Map dimensions: %ux%u pixels", width, height);
    ImGui::Text("Total map data: %zu bytes", mapData.size());
}

// BinaryResourceViewer implementation
void BinaryResourceViewer::SetResource(const std::shared_ptr<ResourceItem>& resource) {
    this->resource = resource;
    dataLoaded = false;
}

void BinaryResourceViewer::SetGameFilePath(const std::string& filePath) {
    gameFilePath = filePath;
    dataLoaded = false;
}

void BinaryResourceViewer::ClearCache() {
    cachedData.clear();
    dataLoaded = false;
}

std::vector<uint8_t> BinaryResourceViewer::LoadBinaryData() {
    if (dataLoaded) return cachedData;
    
    try {
        if (resource->sourceFile.empty()) {
            return {};
        }
        
        BinaryFile file(resource->sourceFile);
        if (!file.IsOpen()) {
            return {};
        }
        
        file.SetPosition(resource->offset);
        std::vector<uint8_t> data;
        data.reserve(resource->size);
        
        for (uint32_t i = 0; i < resource->size && file.GetPosition() < file.GetLength(); ++i) {
            data.push_back(file.ReadByteUnsigned());
        }
        
        cachedData = data;
        dataLoaded = true;
        return data;
        
    } catch (const std::exception& e) {
        std::cerr << "Error reading binary data: " << e.what() << std::endl;
        return {};
    }
}

void BinaryResourceViewer::RenderHexDump(const std::vector<uint8_t>& data, size_t maxBytes) {
    ImGui::Text("Hex Dump (first %zu bytes):", maxBytes);
    
    const size_t bytesToShow = std::min(data.size(), maxBytes);
    const size_t bytesPerLine = 16;
    
    for (size_t i = 0; i < bytesToShow; i += bytesPerLine) {
        // Hex values
        std::string hexLine;
        std::string asciiLine;
        
        for (size_t j = 0; j < bytesPerLine && (i + j) < bytesToShow; ++j) {
            unsigned char byte = data[i + j];
            char hexByte[4];
            snprintf(hexByte, sizeof(hexByte), "%02X ", byte);
            hexLine += hexByte;
            
            // ASCII representation
            if (byte >= 32 && byte <= 126) {
                asciiLine += static_cast<char>(byte);
            } else {
                asciiLine += '.';
            }
        }
        
        ImGui::Text("%04zX: %-48s |%s|", i, hexLine.c_str(), asciiLine.c_str());
    }
    
    if (data.size() > maxBytes) {
        ImGui::Text("... (truncated)");
    }
}

void BinaryResourceViewer::RenderProperties() {
    if (!resource) {
        ImGui::Text("No resource selected");
        return;
    }
    
    ImGui::Text("Binary Resource Properties");
    ImGui::Separator();
    
    ImGui::Text("Name: %s", resource->name.c_str());
    ImGui::Text("Type: %s", GetResourceTypeString(resource->type));
    ImGui::Text("Offset: 0x%08X", resource->offset);
    ImGui::Text("Size: %u bytes", resource->size);
    ImGui::Separator();
    ImGui::Text("Size Details:");
    ImGui::Text("  KB: %.2f", resource->size / 1024.0f);
    ImGui::Text("  MB: %.4f", resource->size / (1024.0f * 1024.0f));
    ImGui::Separator();
    
    std::vector<uint8_t> data = LoadBinaryData();
    if (!data.empty()) {
        RenderHexDump(data);
    } else {
        ImGui::Text("(Failed to read binary data)");
    }
}

void BinaryResourceViewer::RenderPreview() {
    if (!resource) {
        ImGui::Text("No resource selected");
        return;
    }
    
    ImGui::Text("Binary Data Viewer");
    ImGui::Separator();
    
    ImGui::Text("Resource: %s", resource->name.c_str());
    ImGui::Text("Type: %s", GetResourceTypeString(resource->type));
    ImGui::Text("Size: %u bytes", resource->size);
    ImGui::Separator();
    
    std::vector<uint8_t> data = LoadBinaryData();
    if (!data.empty()) {
        ImGui::Text("Binary Data (first 256 bytes):");
        RenderHexDump(data, 256);  // Show more data in preview
        
        ImGui::Separator();
        ImGui::Text("Data Analysis:");
        ImGui::Text("Total bytes: %zu", data.size());
        
        // Count different byte values
        std::map<uint8_t, size_t> byteCounts;
        for (uint8_t byte : data) {
            byteCounts[byte]++;
        }
        
        ImGui::Text("Unique byte values: %zu", byteCounts.size());
        ImGui::Text("Most common byte: 0x%02X (%zu occurrences)", 
                   byteCounts.begin()->first, byteCounts.begin()->second);
    } else {
        ImGui::Text("(Failed to read binary data)");
    }
}

// CharResourceViewer implementation
void CharResourceViewer::SetResource(const std::shared_ptr<ResourceItem>& resource) {
    this->resource = resource;
    dataLoaded = false;
}

void CharResourceViewer::SetGameFilePath(const std::string& filePath) {
    gameFilePath = filePath;
    dataLoaded = false;
}

void CharResourceViewer::ClearCache() {
    cachedDecompressedData.clear();
    dataLoaded = false;
}

std::vector<uint8_t> CharResourceViewer::DecompressTileData() {
    if (dataLoaded) return cachedDecompressedData;
    
    try {
        if (resource->sourceFile.empty()) {
            return {};
        }
        
        BinaryFile file(resource->sourceFile);
        if (!file.IsOpen()) {
            return {};
        }
        
        // CHAR specific: DataStartOffset = offset + 4, read all tiles
        uint32_t dataStartOffset = resource->offset + 4;
        uint32_t expectedSize = TILE_COUNT * TILE_BYTES;
        
        if (dataStartOffset >= file.GetLength()) {
            return {};
        }
        
        file.SetPosition(dataStartOffset);
        
        // Read all tile data
        std::vector<uint8_t> tileData;
        tileData.reserve(expectedSize);
        
        for (uint32_t i = 0; i < expectedSize && file.GetPosition() < file.GetLength(); ++i) {
            tileData.push_back(file.ReadByteUnsigned());
        }
        
        cachedDecompressedData = tileData;
        dataLoaded = true;
        return tileData;
        
    } catch (const std::exception& e) {
        std::cerr << "Error reading tile data: " << e.what() << std::endl;
        return {};
    }
}

std::vector<uint8_t> CharResourceViewer::DecodeTile(const std::vector<uint8_t>& tileData, size_t tileIndex) {
    std::vector<uint8_t> decodedTile(TILE_SIZE * TILE_SIZE, 0);
    
    size_t tileOffset = tileIndex * TILE_BYTES;
    if (tileOffset + TILE_BYTES > tileData.size()) {
        return decodedTile;
    }
    
    // Decode 4bpp tile data (2 pixels per byte)
    size_t pixelIndex = 0;
    for (size_t i = 0; i < TILE_BYTES && pixelIndex < TILE_SIZE * TILE_SIZE; ++i) {
        uint8_t byte = tileData[tileOffset + i];
        uint8_t pixel1 = byte & 0x0F;  // Lower nibble
        uint8_t pixel2 = (byte >> 4) & 0x0F;  // Upper nibble
        
        decodedTile[pixelIndex++] = pixel1;
        if (pixelIndex < TILE_SIZE * TILE_SIZE) {
            decodedTile[pixelIndex++] = pixel2;
        }
    }
    
    return decodedTile;
}

ImU32 CharResourceViewer::GetTileColor(uint8_t pixelValue) {
    // Use the palette to get the correct color
    return GetPaletteColor(pixelValue);
}

void CharResourceViewer::RenderProperties() {
    if (!resource) {
        ImGui::Text("No resource selected");
        return;
    }
    
    ImGui::Text("Tile Resource Properties");
    ImGui::Separator();
    
    ImGui::Text("Name: %s", resource->name.c_str());
    ImGui::Text("Type: Character/Tile (CHAR)");
    ImGui::Text("Offset: 0x%08X", resource->offset);
    ImGui::Text("Size: %u bytes", resource->size);
    ImGui::Separator();
    
    std::vector<uint8_t> tileData = DecompressTileData();
    if (!tileData.empty()) {
        ImGui::Text("Tile Information:");
        ImGui::Text("  Total tiles: %u", TILE_COUNT);
        ImGui::Text("  Tile size: %ux%u pixels", TILE_SIZE, TILE_SIZE);
        ImGui::Text("  Bytes per tile: %u", TILE_BYTES);
        ImGui::Text("  Total tile data: %zu bytes", tileData.size());
        ImGui::Text("  Format: 4bpp (2 pixels per byte)");
    } else {
        ImGui::Text("(Failed to read tile data)");
    }
}

void CharResourceViewer::RenderPreview() {
    if (!resource) {
        ImGui::Text("No resource selected");
        return;
    }
    
    ImGui::Text("Tile Sheet Viewer");
    ImGui::Separator();
    
    std::vector<uint8_t> tileData = DecompressTileData();
    if (tileData.empty()) {
        ImGui::Text("(Failed to read tile data)");
        return;
    }
    
    ImGui::Text("Tile Sheet (%u tiles, %ux%u pixels each):", TILE_COUNT, TILE_SIZE, TILE_SIZE);
    ImGui::Separator();
    
    // Calculate tile sheet layout (16x16 tiles = 256 tiles)
    const int tilesPerRow = 16;
    const int tilesPerCol = 16;
    const int tileScale = 2; // 2x2 pixels per tile for visibility
    
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Draw the tile sheet
    for (int row = 0; row < tilesPerCol; ++row) {
        for (int col = 0; col < tilesPerRow; ++col) {
            int tileIndex = row * tilesPerRow + col;
            if (tileIndex < TILE_COUNT) {
                std::vector<uint8_t> decodedTile = DecodeTile(tileData, tileIndex);
                
                // Draw the tile
                for (int y = 0; y < TILE_SIZE; ++y) {
                    for (int x = 0; x < TILE_SIZE; ++x) {
                        size_t pixelIndex = y * TILE_SIZE + x;
                        if (pixelIndex < decodedTile.size()) {
                            ImU32 color = GetTileColor(decodedTile[pixelIndex]);
                            ImVec2 p0 = ImVec2(
                                canvasPos.x + (col * TILE_SIZE + x) * tileScale,
                                canvasPos.y + (row * TILE_SIZE + y) * tileScale
                            );
                            ImVec2 p1 = ImVec2(p0.x + tileScale, p0.y + tileScale);
                            drawList->AddRectFilled(p0, p1, color);
                        }
                    }
                }
            }
        }
    }
    
    // Reserve space in ImGui layout for the drawn area
    ImGui::Dummy(ImVec2(tilesPerRow * TILE_SIZE * tileScale, tilesPerCol * TILE_SIZE * tileScale));
    ImGui::Separator();
    ImGui::Text("Tile Information:");
    ImGui::Text("  Total tiles: %u", TILE_COUNT);
    ImGui::Text("  Tile size: %ux%u pixels", TILE_SIZE, TILE_SIZE);
    ImGui::Text("  Bytes per tile: %u", TILE_BYTES);
    ImGui::Text("  Total tile data: %zu bytes", tileData.size());
    ImGui::Text("  Display scale: %dx%d pixels per tile", tileScale, tileScale);
}



// Static member initialization
std::vector<ImU32> MapResourceViewer::cachedMapImage;
std::vector<uint8_t> MapResourceViewer::lastMapData;
std::vector<uint8_t> MapResourceViewer::lastTileData;
bool MapResourceViewer::imageValid = false;

void MapResourceViewer::RenderMapWithTiles(const std::vector<uint8_t>& mapData, const std::vector<uint8_t>& tileData) {
    if (mapData.empty() || tileData.empty()) {
        ImGui::Text("(No map data or tile data available)");
        return;
    }
    
    ImGui::Text("Map with Actual Tiles:");
    ImGui::Separator();
    
    // Check if we need to regenerate the image
    if (!imageValid || mapData != lastMapData || tileData != lastTileData) {
        RenderMapToImage(mapData, tileData);
        lastMapData = mapData;
        lastTileData = tileData;
        imageValid = true;
    }
    
    // Display the cached image
    if (!cachedMapImage.empty()) {
        const int imageWidth = mapGridWidth * TILE_SIZE;
        const int imageHeight = mapGridHeight * TILE_SIZE;
        const int scale = 4; // 4x scale
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Draw the cached image pixels at 4x size
        for (int y = 0; y < imageHeight; ++y) {
            for (int x = 0; x < imageWidth; ++x) {
                size_t imageIndex = y * imageWidth + x;
                if (imageIndex < cachedMapImage.size()) {
                    ImU32 color = cachedMapImage[imageIndex];
                    ImVec2 p0 = ImVec2(canvasPos.x + x * scale, canvasPos.y + y * scale);
                    ImVec2 p1 = ImVec2(p0.x + scale, p0.y + scale);
                    drawList->AddRectFilled(p0, p1, color);
                }
            }
        }
        
        // Reserve space in ImGui layout for the drawn area
        ImGui::Dummy(ImVec2(imageWidth * scale, imageHeight * scale));
    }
}

void MapResourceViewer::RenderMapToImage(const std::vector<uint8_t>& mapData, const std::vector<uint8_t>& tileData) {
    // Pre-decode all tiles once
    std::vector<std::vector<uint8_t>> tileCache;
    tileCache.reserve(256);
    for (int i = 0; i < 256; ++i) {
        tileCache.push_back(CharResourceViewer::DecodeTile(tileData, i));
    }
    
    // Create image buffer (16x16 pixels per map tile)
    const int imageWidth = mapGridWidth * TILE_SIZE;
    const int imageHeight = mapGridHeight * TILE_SIZE;
    cachedMapImage.clear();
    cachedMapImage.resize(imageWidth * imageHeight, IM_COL32(0, 0, 0, 255)); // Initialize with black
    
    // Render map to image buffer (16x16 pixels per tile)
    for (uint32_t mapRow = 0; mapRow < mapGridHeight; ++mapRow) {
        for (uint32_t mapCol = 0; mapCol < mapGridWidth; ++mapCol) {
            size_t mapIndex = mapRow * mapGridWidth + mapCol;
            if (mapIndex < mapData.size()) {
                uint8_t tileIndex = mapData[mapIndex];
                if (tileIndex < 256 && tileIndex < tileCache.size()) {
                    const std::vector<uint8_t>& decodedTile = tileCache[tileIndex];
                    
                    // Calculate the position of this tile in the image
                    int imageStartX = mapCol * TILE_SIZE;
                    int imageStartY = mapRow * TILE_SIZE;
                    
                    // Render the full 16x16 tile
                    for (int tileY = 0; tileY < TILE_SIZE; ++tileY) {
                        for (int tileX = 0; tileX < TILE_SIZE; ++tileX) {
                            size_t pixelIndex = tileY * TILE_SIZE + tileX;
                            if (pixelIndex < decodedTile.size()) {
                                uint8_t pixelValue = decodedTile[pixelIndex];
                                ImU32 color = CharResourceViewer::GetTileColor(pixelValue);
                                
                                // Calculate the position in the image buffer
                                int imageX = imageStartX + tileX;
                                int imageY = imageStartY + tileY;
                                size_t imageIndex = imageY * imageWidth + imageX;
                                
                                if (imageIndex < cachedMapImage.size()) {
                                    cachedMapImage[imageIndex] = color;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}



// Helper function for resource type strings
const char* GetResourceTypeString(ResourceType type) {
    switch (type) {
        case ResourceType::CHAR: return "Character";
        case ResourceType::CSTR: return "String";
        case ResourceType::FONT: return "Font";
        case ResourceType::FRML: return "Form";
        case ResourceType::IMAG: return "Image";
        case ResourceType::MMAP: return "Map";
        case ResourceType::ARCHIVE: return "Archive";
        default: return "Unknown";
    }
} 

// Static method implementation for CharResourceViewer
std::vector<uint8_t> CharResourceViewer::GetTileData(const std::string& sourceFile, uint32_t offset) {
    constexpr uint32_t TILE_COUNT = 256;
    constexpr uint32_t TILE_BYTES = 128;
    try {
        BinaryFile file(sourceFile);
        if (!file.IsOpen()) {
            return {};
        }
        uint32_t dataStartOffset = offset + 4;
        uint32_t expectedSize = TILE_COUNT * TILE_BYTES;
        if (dataStartOffset >= file.GetLength()) {
            return {};
        }
        file.SetPosition(dataStartOffset);
        std::vector<uint8_t> tileData;
        tileData.reserve(expectedSize);
        for (uint32_t i = 0; i < expectedSize && file.GetPosition() < file.GetLength(); ++i) {
            tileData.push_back(file.ReadByteUnsigned());
        }
        return tileData;
    } catch (const std::exception& e) {
        std::cerr << "Error reading tile data: " << e.what() << std::endl;
        return {};
    }
} 