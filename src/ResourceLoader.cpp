#include "ResourceLoader.h"
#include <iostream>
#include <filesystem>

std::unique_ptr<ResourceIndex> ResourceLoader::LoadResourceFile(const std::string& filename, Endianness endian) {
    try {
        BinaryFile file(filename);
        if (!file.IsOpen()) {
            std::cerr << "Failed to open resource file: " << filename << std::endl;
            return nullptr;
        }
        
        // Read header
        ResourceHeader header = ReadResourceHeader(file, endian);
        std::cout << "Resource file size: " << header.size << " bytes" << std::endl;
        
        // Calculate file pointer position
        uint32_t filePointer = (header.dataSegmentSize + header.size) + 14;
        
        // Read resource identifiers
        auto identifiers = ReadResourceIdentifiers(file, filePointer, endian);
        std::cout << "Found " << identifiers.size() << " resource types" << std::endl;
        
        // Get key position
        uint32_t keyPosition = GetResourceKeyPosition(file, endian);
        
        // Create resource index
        auto resourceIndex = std::make_unique<ResourceIndex>("WIME");
        std::string stripFilename = std::filesystem::path(filename).stem().string();
        
        uint32_t tk = 0;
        for (const auto& identifier : identifiers) {
            std::cout << "Processing " << identifier.resourceID << " with " << identifier.resourceQTY << " items" << std::endl;
            
            // Read resource maps for this type
            auto maps = ReadResourceMaps(file, keyPosition, identifier.resourceQTY, endian);
            
            for (const auto& map : maps) {
                // Calculate actual offset
                uint32_t actualOffset = (map.offset + header.size) + (65535 * map.multiplier) + (1 * map.multiplier);
                
                // Get chunk size
                uint32_t chunkSize = GetChunkSize(file, actualOffset, endian);
                
                // Create resource item
                std::string resourceName = identifier.resourceID + " " + std::to_string(map.number);
                ResourceType resourceType = GetResourceType(identifier.resourceID);
                
                resourceIndex->AddItem(resourceName, actualOffset, chunkSize, resourceType);
                
                tk++;
            }
        }
        
        std::cout << "Loaded " << resourceIndex->items.size() << " resources from " << filename << std::endl;
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

std::vector<ResourceIdentifier> ResourceLoader::ReadResourceIdentifiers(BinaryFile& file, uint32_t filePointer, Endianness endian) {
    std::vector<ResourceIdentifier> identifiers;
    
    file.SetPosition(filePointer);
    
    // Read first identifier to determine count
    std::string firstID = GetChunkID(file, filePointer, endian);
    if (firstID.empty()) {
        return identifiers;
    }
    
    uint32_t currentPos = filePointer;
    while (true) {
        std::string resourceID = GetChunkID(file, currentPos, endian);
        if (resourceID.empty()) break;
        
        currentPos += 4;
        uint16_t resourceQTY = GetChunkQTY(file, currentPos, endian);
        currentPos += 4;
        
        identifiers.push_back({resourceID, resourceQTY});
    }
    
    return identifiers;
}

std::vector<ResourceMap> ResourceLoader::ReadResourceMaps(BinaryFile& file, uint32_t keyPosition, uint16_t count, Endianness endian) {
    std::vector<ResourceMap> maps;
    
    for (uint16_t i = 0; i < count; i++) {
        uint32_t offset = keyPosition + (12 * i);
        
        ResourceMap map;
        map.number = ReadResMapNum(file, offset, endian);
        map.offset = ReadResMapOffset(file, offset + 4, endian);
        map.multiplier = ReadResMapMultiplier(file, offset + 6, endian);
        
        maps.push_back(map);
    }
    
    return maps;
}

uint32_t ResourceLoader::GetResourceKeyPosition(BinaryFile& file, Endianness endian) {
    // This is a simplified version - in the real implementation, 
    // this would calculate the key position based on the file structure
    file.SetPosition(0);
    uint32_t headerSize = file.ReadLongwordUnsigned(endian);
    return headerSize + 14; // Simplified calculation
}

std::string ResourceLoader::GetChunkID(BinaryFile& file, uint32_t offset, Endianness endian) {
    file.SetPosition(offset);
    
    std::string id;
    for (int i = 0; i < 4; i++) {
        char c = static_cast<char>(file.ReadByteUnsigned());
        if (c == 0) break;
        id += c;
    }
    
    return id;
}

uint16_t ResourceLoader::GetChunkQTY(BinaryFile& file, uint32_t offset, Endianness endian) {
    file.SetPosition(offset);
    return file.ReadWordUnsigned(endian);
}

uint16_t ResourceLoader::ReadResMapNum(BinaryFile& file, uint32_t offset, Endianness endian) {
    file.SetPosition(offset);
    return file.ReadWordUnsigned(endian);
}

uint32_t ResourceLoader::ReadResMapOffset(BinaryFile& file, uint32_t offset, Endianness endian) {
    file.SetPosition(offset);
    return file.ReadLongwordUnsigned(endian);
}

uint16_t ResourceLoader::ReadResMapMultiplier(BinaryFile& file, uint32_t offset, Endianness endian) {
    file.SetPosition(offset);
    return file.ReadWordUnsigned(endian);
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