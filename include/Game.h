#pragma once
#include <string>
#include <vector>
#include <memory>
#include "FileFormat.h"
#include "BinaryFile.h"
#include "ResourceIndex.h"

// Game format types
enum class GameFormat {
    Unknown,
    PC,
    Amiga,
    AppleIIGS,
    AtariST
};

class Game {
public:
    std::string Name;
    std::string FilePath;
    bool IsLoaded;
    GameFormat format = GameFormat::Unknown;
    std::unique_ptr<ResourceIndex> resource;
    std::unique_ptr<FileFormat> fileFormat;

    Game() : IsLoaded(false) {}
    
    bool LoadGame(const std::string& filePath);
    void UnloadGame();
    
private:
    bool InitializeGameData(const std::string& filePath);
    GameFormat DetectFormat(const std::string& filePath, BinaryFile& file);
    void LoadRealResources(const std::string& gamePath);
}; 