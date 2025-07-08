#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

enum class Endianness {
    Little,
    Big
};

class BinaryFile {
public:
    BinaryFile(const std::string& filename);
    ~BinaryFile();
    
    // File operations
    bool IsOpen() const;
    std::string GetFilename() const;
    size_t GetPosition(); // removed const
    void SetPosition(size_t position);
    size_t GetLength(); // removed const
    
    // Byte operations
    uint8_t ReadByteUnsigned();
    int8_t ReadByteSigned();
    void WriteByteUnsigned(uint8_t value);
    void WriteByteSigned(int8_t value);
    
    // Word operations (16-bit)
    int16_t ReadWordSigned(Endianness endian = Endianness::Little);
    uint16_t ReadWordUnsigned(Endianness endian = Endianness::Little);
    void WriteWordSigned(int16_t value, Endianness endian = Endianness::Little);
    void WriteWordUnsigned(uint16_t value, Endianness endian = Endianness::Little);
    
    // Longword operations (32-bit)
    int32_t ReadLongwordSigned(Endianness endian = Endianness::Little);
    uint32_t ReadLongwordUnsigned(Endianness endian = Endianness::Little);
    void WriteLongwordSigned(int32_t value, Endianness endian = Endianness::Little);
    void WriteLongwordUnsigned(uint32_t value, Endianness endian = Endianness::Little);
    
    // String operations
    std::string ReadString(size_t length);
    void WriteString(const std::string& value);
    
    // Utility functions
    static void SwapWord(int16_t& word);
    static void SwapLongword(int32_t& longword);
    static void Nibbler(uint16_t value, uint8_t& byte1, uint8_t& byte2);
    static int16_t ReadShort(uint8_t byte1, uint8_t byte2, Endianness endian = Endianness::Little);
    
    // Array operations
    std::vector<uint8_t> ReadBytes(size_t count);
    void WriteBytes(const std::vector<uint8_t>& data);

private:
    std::fstream file;
    std::string filename;
    
    // Helper functions
    void CheckEOF();
    uint16_t SwapBytes(uint16_t value);
    uint32_t SwapBytes(uint32_t value);
}; 