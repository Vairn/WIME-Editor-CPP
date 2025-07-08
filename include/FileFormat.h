#pragma once
#include <string>

class FileFormat {
public:
    std::string Name;
    std::string Endian;
    std::string DataEndian;
    std::string ExecutableFile;
    std::string Icon;
    int BitPlanes;
    int FRMLBitplanes;

    FileFormat() = default;
    
    FileFormat(const std::string& name, const std::string& endian, 
               const std::string& dataEndian, const std::string& executableFile,
               const std::string& icon, int bitPlanes, int frmlBitplanes)
        : Name(name), Endian(endian), DataEndian(dataEndian), 
          ExecutableFile(executableFile), Icon(icon), BitPlanes(bitPlanes), 
          FRMLBitplanes(frmlBitplanes) {}
}; 