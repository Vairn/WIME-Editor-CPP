#include "Game.h"
#include "FileFormat.h"
#include "BinaryFile.h"
#include "ResourceLoader.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

static std::string ToLower(const std::string& str) {
    std::string out = str;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

bool Game::LoadGame(const std::string& filePath) {
    try {
        FilePath = filePath;
        Name = std::filesystem::path(filePath).filename().string();
        
        if (InitializeGameData(filePath)) {
            IsLoaded = true;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading game: " << e.what() << std::endl;
    }
    
    return false;
}

void Game::UnloadGame() {
    IsLoaded = false;
    Name.clear();
    FilePath.clear();
    resource.reset();
    fileFormat.reset();
    format = GameFormat::Unknown;
}

GameFormat Game::DetectFormat(const std::string& filePath, BinaryFile& file) {
    std::string fname = ToLower(std::filesystem::path(filePath).filename().string());
    if (fname == "start.exe" || fname == "lord.exe") return GameFormat::PC;
    if (fname.find("earth.sys16") != std::string::npos) return GameFormat::AppleIIGS;
    if (fname.find("warinmiddleearth") != std::string::npos) return GameFormat::Amiga;
    if (fname == "command.prg") return GameFormat::AtariST;
    // Optionally, check file header bytes for more robust detection
    // ...
    return GameFormat::Unknown;
}

void Game::LoadRealResources(const std::string& gamePath) {
    try {
        // Determine endianness based on format
        Endianness endian = (format == GameFormat::PC || format == GameFormat::AppleIIGS) ? 
                           Endianness::Little : Endianness::Big;
        
        // Get the directory containing the game file
        std::filesystem::path gameFilePath(gamePath);
        std::string gameDir = gameFilePath.parent_path().string();
        
        std::cout << "Looking for .res files in: " << gameDir << std::endl;
        
        // Look for .res files in the same directory
        std::vector<std::string> resourceFiles;
        for (const auto& entry : std::filesystem::directory_iterator(gameDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".res") {
                std::string resFile = entry.path().string();
                std::cout << "Found resource file: " << resFile << std::endl;
                
                // Try to load the resource file
                auto loadedResource = ResourceLoader::LoadResourceFile(resFile, endian);
                if (loadedResource) {
                    // Merge with existing resources or replace
                    if (!resource) {
                        resource = std::move(loadedResource);
                    } else {
                        // Merge resources
                        for (const auto& item : loadedResource->items) {
                            resource->AddItem(item.name, item.offset, item.size, item.type);
                        }
                    }
                    std::cout << "Successfully loaded resources from: " << resFile << std::endl;
                } else {
                    std::cout << "Failed to load resources from: " << resFile << std::endl;
                }
            }
        }
        
        if (!resource) {
            std::cout << "No valid resource files found, creating empty resource index" << std::endl;
            resource = std::make_unique<ResourceIndex>("WIME");
        }
        
        std::cout << "Total resources loaded: " << resource->items.size() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading real resources: " << e.what() << std::endl;
        // Fallback to empty resource index
        resource = std::make_unique<ResourceIndex>("WIME");
    }
}

bool Game::InitializeGameData(const std::string& filePath) {
    try {
        BinaryFile file(filePath);
        if (!file.IsOpen()) {
            std::cerr << "Failed to open game file: " << filePath << std::endl;
            return false;
        }
        format = DetectFormat(filePath, file);
        std::string formatStr;
        switch (format) {
            case GameFormat::PC: formatStr = "PC"; break;
            case GameFormat::Amiga: formatStr = "Amiga"; break;
            case GameFormat::AppleIIGS: formatStr = "Apple IIGS"; break;
            case GameFormat::AtariST: formatStr = "Atari ST"; break;
            default: formatStr = "Unknown"; break;
        }
        std::cout << "Detected game format: " << formatStr << std::endl;
        
        // Load real resources from .res files
        LoadRealResources(filePath);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing game data: " << e.what() << std::endl;
        return false;
    }
} 