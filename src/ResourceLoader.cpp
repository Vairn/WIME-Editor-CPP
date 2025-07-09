#include "ResourceLoader.h"
#include <iostream>
#include <filesystem>


std::function<void(const std::string&)> ResourceLoader::debugCallback = nullptr;

void ResourceLoader::SetDebugCallback(std::function<void(const std::string&)> callback) {
    debugCallback = callback;
}

std::unique_ptr<ResourceIndex> ResourceLoader::LoadResourceFile(const std::string& filename, Endianness endian) {
    try {
        BinaryFile file(filename);
        if (!file.IsOpen()) {
            if (debugCallback) debugCallback("Failed to open resource file: " + filename);
            return nullptr;
        }
        
        if (debugCallback) debugCallback("File opened successfully, size: " + std::to_string(file.GetLength()) + " bytes");
        
        // Read header (try both endianness for debug)
        ResourceHeader headerLE = ReadResourceHeader(file, Endianness::Little);
        if (debugCallback) debugCallback("[DEBUG] Header (Little Endian): size=" + std::to_string(headerLE.size) + ", dataSegmentSize=" + std::to_string(headerLE.dataSegmentSize) + ", dataSize=" + std::to_string(headerLE.dataSize) + ", fileEndLength=" + std::to_string(headerLE.fileEndLength));
        ResourceHeader headerBE = ReadResourceHeader(file, Endianness::Big);
        if (debugCallback) debugCallback("[DEBUG] Header (Big Endian): size=" + std::to_string(headerBE.size) + ", dataSegmentSize=" + std::to_string(headerBE.dataSegmentSize) + ", dataSize=" + std::to_string(headerBE.dataSize) + ", fileEndLength=" + std::to_string(headerBE.fileEndLength));
        // Use the original logic for now
        ResourceHeader header = ReadResourceHeader(file, endian);
        if (debugCallback) debugCallback("Resource file size: " + std::to_string(header.size) + " bytes");
        
        // Calculate file pointer position
        uint32_t filePointer = (header.dataSegmentSize + header.size) + 14;
        
        // chunk type quantity
        uint32_t chunkTypePtr = (header.dataSegmentSize + header.size) + 12;
        file.SetPosition(chunkTypePtr);
        uint16_t chunkTypeQty = file.ReadWordUnsigned(endian) + 1;
        if (debugCallback) debugCallback("ChunkTypeQty=" + std::to_string(chunkTypeQty));
        auto identifiers = ReadResourceIdentifiers(file, filePointer, chunkTypeQty, endian);
        
        // Get key position
        uint32_t keyPosition = GetResourceKeyPosition(file, endian);
        
        // Create resource index
        auto resourceIndex = std::make_unique<ResourceIndex>("WIME");
        std::string stripFilename = std::filesystem::path(filename).stem().string();
        
        uint32_t tk = 0;
        for (const auto& identifier : identifiers) {
            if (debugCallback) debugCallback("Processing " + identifier.resourceID + " with " + std::to_string(identifier.resourceQTY) + " items");

            uint32_t identifierKeyStart = keyPosition + (12 * tk); // 12-byte entries as in VB

            auto maps = ReadResourceMaps(file, identifierKeyStart, identifier.resourceQTY, endian);

            for (const auto& map : maps) {
                // Calculate actual offset
                uint32_t actualOffset = (static_cast<uint32_t>(map.offset) + header.size)
                                        + (65535 * static_cast<uint32_t>(map.multiplier))
                                        + (1 * static_cast<uint32_t>(map.multiplier));

                if (debugCallback) debugCallback("  Map: number=" + std::to_string(map.number) + ", offset=" + std::to_string(map.offset) + ", multiplier=" + std::to_string(map.multiplier) + ", actualOffset=" + std::to_string(actualOffset) + ", fileSize=" + std::to_string(file.GetLength()));

                uint32_t chunkSize = 0;
                if (actualOffset < file.GetLength()) {
                    chunkSize = GetChunkSize(file, actualOffset, endian);
                    if (debugCallback) debugCallback("    Chunk size at actualOffset: " + std::to_string(chunkSize));
                } else {
                    if (debugCallback) debugCallback("    actualOffset is past end of file!");
                }

                std::string resourceName = identifier.resourceID + " " + std::to_string(map.number);
                ResourceType resourceType = GetResourceType(identifier.resourceID);
                resourceIndex->AddItem(resourceName, actualOffset, chunkSize, resourceType, filename);

                tk++;
            }
        }
        
        if (debugCallback) debugCallback("Loaded " + std::to_string(resourceIndex->items.size()) + " resources from " + filename);
        return resourceIndex;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading resource file: " << e.what() << std::endl;
        return nullptr;
    }
}

ResourceHeader ResourceLoader::ReadResourceHeader(BinaryFile& file, Endianness endian) {
    file.SetPosition(0);
    
    ResourceHeader header;
    header.size = file.ReadLongwordUnsigned(endian);
    header.dataSegmentSize = file.ReadLongwordUnsigned(endian);
    header.dataSize = file.ReadLongwordUnsigned(endian);
    header.fileEndLength = file.ReadLongwordUnsigned(endian);
    
    return header;
}

std::vector<ResourceIdentifier> ResourceLoader::ReadResourceIdentifiers(BinaryFile& file, uint32_t filePointer, uint16_t expectedCount, Endianness endian) {
    std::vector<ResourceIdentifier> identifiers;
    
    if (debugCallback) debugCallback("ReadResourceIdentifiers: starting at position " + std::to_string(filePointer));
    
    file.SetPosition(filePointer);
    
    for(uint16_t idx=0; idx<expectedCount; ++idx){
        std::string resourceID = GetChunkID(file, filePointer + idx*8, endian);
        uint16_t qty = GetChunkQTY(file, filePointer + idx*8 +4, endian);
        if(resourceID.empty()){
            if (debugCallback) debugCallback("ReadResourceIdentifiers: empty id encountered at index " + std::to_string(idx));
            break;
        }

        identifiers.push_back({resourceID, qty});
    }
    if (debugCallback) debugCallback("ReadResourceIdentifiers: found " + std::to_string(identifiers.size()) + " identifiers");
    return identifiers;
}

std::vector<ResourceMap> ResourceLoader::ReadResourceMaps(BinaryFile& file, uint32_t keyPosition, uint16_t count, Endianness endian) {
    std::vector<ResourceMap> maps;
    
    if (debugCallback) debugCallback("ReadResourceMaps: keyPosition=" + std::to_string(keyPosition) + ", count=" + std::to_string(count) + ", fileSize=" + std::to_string(file.GetLength()));
    
    for (uint16_t i = 0; i < count; i++) {
        uint32_t offset = keyPosition + (12 * i); // 12-byte map entries
        
        if (debugCallback) debugCallback("ReadResourceMaps: reading map " + std::to_string(i) + " at offset " + std::to_string(offset));
        
        // Check if offset is valid
        if (offset >= file.GetLength()) {
            if (debugCallback) debugCallback("ReadResourceMaps: offset " + std::to_string(offset) + " is past end of file!");
            break;
        }
        
        ResourceMap map;
        map.number = ReadResMapNum(file, offset, endian);
        map.offset = ReadResMapOffset(file, offset + 4, endian);
        map.multiplier = ReadResMapMultiplier(file, offset + 6, endian);
        
        if (debugCallback) debugCallback("ReadResourceMaps: map " + std::to_string(i) + " = number:" + std::to_string(map.number) + ", offset:" + std::to_string(map.offset) + ", multiplier:" + std::to_string(map.multiplier));
        
        maps.push_back(map);
    }
    
    return maps;
}

uint32_t ResourceLoader::GetResourceKeyPosition(BinaryFile& file, Endianness endian) {
    // Replicate VB.NET logic to find resource map key start position
    file.SetPosition(0);
    uint32_t headerSize = file.ReadLongwordUnsigned(endian);           // header.size
    uint32_t dataSegmentSize = file.ReadLongwordUnsigned(endian);     // header.dataSegmentSize

    // Position after header fields
    uint32_t chunkTypePtr = (dataSegmentSize + headerSize) + 12;      // points to chunk type quantity word

    file.SetPosition(chunkTypePtr);
    uint16_t chunkQty = file.ReadWordUnsigned(endian);
    chunkQty += 1; // VB adds 1

    uint32_t keyPosition = (dataSegmentSize + headerSize) + 14 + (8 * chunkQty);
    return keyPosition;
}

std::string ResourceLoader::GetChunkID(BinaryFile& file, uint32_t offset, Endianness endian) {
    if (debugCallback) debugCallback("GetChunkID: reading at offset " + std::to_string(offset));
    
    file.SetPosition(offset);
    
    // Check if we're at end of file
    if (file.GetPosition() >= file.GetLength()) {
        if (debugCallback) debugCallback("GetChunkID: at end of file");
        return "";
    }
    
    // Always read as big-endian (like VB.NET)
    uint32_t idInt = file.ReadLongwordUnsigned(Endianness::Big);
    if (debugCallback) debugCallback("GetChunkID: raw integer = 0x" + std::to_string(idInt));
    
    // Convert to string (4 characters)
    std::string id;
    if (endian == Endianness::Big) {
        // Big-endian: reverse bytes (like VB.NET tempend = 1)
        id += static_cast<char>((idInt >> 24) & 0xFF);
        id += static_cast<char>((idInt >> 16) & 0xFF);
        id += static_cast<char>((idInt >> 8) & 0xFF);
        id += static_cast<char>(idInt & 0xFF);
    } else {
        // Little-endian: forward order (like VB.NET tempend = 0)
        id += static_cast<char>(idInt & 0xFF);
        id += static_cast<char>((idInt >> 8) & 0xFF);
        id += static_cast<char>((idInt >> 16) & 0xFF);
        id += static_cast<char>((idInt >> 24) & 0xFF);
    }
    
    // Remove null terminators
    while (!id.empty() && id.back() == '\0') {
        id.pop_back();
    }
    
    if (debugCallback) debugCallback("GetChunkID: returning '" + id + "'");
    return id;
}

uint16_t ResourceLoader::GetChunkQTY(BinaryFile& file, uint32_t offset, Endianness endian) {
    if (debugCallback) debugCallback("GetChunkQTY: reading at offset " + std::to_string(offset));
    file.SetPosition(offset);

    // Check enough bytes
    if (file.GetPosition() + 2 > file.GetLength()) {
        if (debugCallback) debugCallback("GetChunkQTY: not enough bytes to read word");
        return 0;
    }
    uint16_t qty = file.ReadWordUnsigned(endian);
    qty += 1;
    if (debugCallback) debugCallback("GetChunkQTY: returning " + std::to_string(qty));
    return qty;
}

uint16_t ResourceLoader::ReadResMapNum(BinaryFile& file, uint32_t offset, Endianness endian) {
    file.SetPosition(offset);
    return file.ReadWordUnsigned(endian);  // 2 bytes
}

uint16_t ResourceLoader::ReadResMapOffset(BinaryFile& file, uint32_t offset, Endianness endian) {
    uint32_t startOffset = offset;
    if (endian == Endianness::Big) {
        startOffset += 2; // VB shifts +2 for big-endian
    }
    file.SetPosition(startOffset);
    return file.ReadWordUnsigned(endian);
}

uint8_t ResourceLoader::ReadResMapMultiplier(BinaryFile& file, uint32_t offset, Endianness endian) {
    uint32_t startOffset = offset;
    if (endian == Endianness::Big) {
        // VB shifts -1 for big-endian
        if (startOffset > 0) startOffset -= 1;
    }
    file.SetPosition(startOffset);
    return file.ReadByteUnsigned();
}

uint32_t ResourceLoader::GetChunkSize(BinaryFile& file, uint32_t offset, Endianness endian) {
    file.SetPosition(offset);
    return file.ReadLongwordUnsigned(endian);
}

ResourceType ResourceLoader::GetResourceType(const std::string& resourceID) {
    if (resourceID == "CHAR") return ResourceType::CHAR;
    if (resourceID == "CSTR") return ResourceType::CSTR;
    if (resourceID == "FONT") return ResourceType::FONT;
    if (resourceID == "FRML") return ResourceType::FRML;
    if (resourceID == "IMAG") return ResourceType::IMAG;
    if (resourceID == "MMAP") return ResourceType::MMAP;
    return ResourceType::CHAR; // Default
}

bool ResourceLoader::ValidateResourceHeader(const std::string& filename, Endianness endian) {
    try {
        BinaryFile file(filename);
        if (!file.IsOpen()) return false;
        
        // Read header and validate
        ResourceHeader header = ReadResourceHeader(file, endian);
        
        // Basic validation - file should be at least header size
        if (header.size < 16) return false;
        
        return true;
    } catch (...) {
        return false;
    }
} 