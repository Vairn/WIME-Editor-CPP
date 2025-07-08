#pragma once
#include <string>
#include <vector>
#include <memory>
#include "BinaryFile.h"
#include "ResourceIndex.h"

// Resource file header structure
struct ResourceHeader {
    uint32_t size;
    uint32_t dataSegmentSize;
    uint32_t dataSize;
    uint32_t fileEndLength;
};

// Resource map entry structure
struct ResourceMap {
    uint16_t number;
    uint32_t offset;
    uint16_t multiplier;
};

// Resource identifier structure
struct ResourceIdentifier {
    std::string resourceID;
    uint16_t resourceQTY;
};

class ResourceLoader {
public:
    static std::unique_ptr<ResourceIndex> LoadResourceFile(const std::string& filename, Endianness endian);
    
private:
    static ResourceHeader ReadResourceHeader(BinaryFile& file, Endianness endian);
    static std::vector<ResourceIdentifier> ReadResourceIdentifiers(BinaryFile& file, uint32_t filePointer, Endianness endian);
    static std::vector<ResourceMap> ReadResourceMaps(BinaryFile& file, uint32_t keyPosition, uint16_t count, Endianness endian);
    static uint32_t GetResourceKeyPosition(BinaryFile& file, Endianness endian);
    static std::string GetChunkID(BinaryFile& file, uint32_t offset, Endianness endian);
    static uint16_t GetChunkQTY(BinaryFile& file, uint32_t offset, Endianness endian);
    static uint16_t ReadResMapNum(BinaryFile& file, uint32_t offset, Endianness endian);
    static uint32_t ReadResMapOffset(BinaryFile& file, uint32_t offset, Endianness endian);
    static uint16_t ReadResMapMultiplier(BinaryFile& file, uint32_t offset, Endianness endian);
    static uint32_t GetChunkSize(BinaryFile& file, uint32_t offset, Endianness endian);
    static ResourceType GetResourceType(const std::string& resourceID);
    static bool ValidateResourceHeader(const std::string& filename, Endianness endian);
}; 