#include "BinaryFile.h"
#include <iostream>
#include <iomanip>
#include <sstream>

BinaryFile::BinaryFile(const std::string& filename) : filename(filename) {
    file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
}

BinaryFile::~BinaryFile() {
    if (file.is_open()) {
        file.close();
    }
}

bool BinaryFile::IsOpen() const {
    return file.is_open();
}

std::string BinaryFile::GetFilename() const {
    return filename;
}

size_t BinaryFile::GetPosition() {
    return file.tellg();
}

void BinaryFile::SetPosition(size_t position) {
    file.seekg(position);
    file.seekp(position);
}

size_t BinaryFile::GetLength() {
    auto current = file.tellg();
    file.seekg(0, std::ios::end);
    auto length = file.tellg();
    file.seekg(current);
    return length;
}

uint8_t BinaryFile::ReadByteUnsigned() {
    uint8_t value;
    file.read(reinterpret_cast<char*>(&value), 1);
    CheckEOF();
    return value;
}

int8_t BinaryFile::ReadByteSigned() {
    uint8_t byteValue = ReadByteUnsigned();
    int8_t result;
    
    if (byteValue <= 0x7F) {
        result = static_cast<int8_t>(byteValue);
    } else {
        result = static_cast<int8_t>(byteValue - 0x100);
    }
    
    return result;
}

void BinaryFile::WriteByteUnsigned(uint8_t value) {
    file.write(reinterpret_cast<const char*>(&value), 1);
}

void BinaryFile::WriteByteSigned(int8_t value) {
    uint8_t byteToWrite;
    if (value >= 0) {
        byteToWrite = static_cast<uint8_t>(value);
    } else {
        byteToWrite = static_cast<uint8_t>(value + 256);
    }
    WriteByteUnsigned(byteToWrite);
}

int16_t BinaryFile::ReadWordSigned(Endianness endian) {
    uint8_t loByte, hiByte;
    
    if (endian == Endianness::Little) {
        loByte = ReadByteUnsigned();
        hiByte = ReadByteUnsigned();
    } else {
        hiByte = ReadByteUnsigned();
        loByte = ReadByteUnsigned();
    }
    
    return static_cast<int16_t>((hiByte << 8) | loByte);
}

uint16_t BinaryFile::ReadWordUnsigned(Endianness endian) {
    uint8_t loByte, hiByte;
    
    if (endian == Endianness::Little) {
        loByte = ReadByteUnsigned();
        hiByte = ReadByteUnsigned();
    } else {
        hiByte = ReadByteUnsigned();
        loByte = ReadByteUnsigned();
    }
    
    return static_cast<uint16_t>((hiByte << 8) | loByte);
}

void BinaryFile::WriteWordSigned(int16_t value, Endianness endian) {
    uint8_t hiByte = value >> 8;
    uint8_t loByte = value & 0xFF;
    
    if (endian == Endianness::Little) {
        WriteByteUnsigned(loByte);
        WriteByteUnsigned(hiByte);
    } else {
        WriteByteUnsigned(hiByte);
        WriteByteUnsigned(loByte);
    }
}

void BinaryFile::WriteWordUnsigned(uint16_t value, Endianness endian) {
    uint8_t hiByte = value >> 8;
    uint8_t loByte = value & 0xFF;
    
    if (endian == Endianness::Little) {
        WriteByteUnsigned(loByte);
        WriteByteUnsigned(hiByte);
    } else {
        WriteByteUnsigned(hiByte);
        WriteByteUnsigned(loByte);
    }
}

int32_t BinaryFile::ReadLongwordSigned(Endianness endian) {
    int16_t loWord, hiWord;
    
    if (endian == Endianness::Little) {
        loWord = ReadWordSigned(endian);
        hiWord = ReadWordSigned(endian);
    } else {
        hiWord = ReadWordSigned(endian);
        loWord = ReadWordSigned(endian);
    }
    
    return static_cast<int32_t>((hiWord << 16) | loWord);
}

uint32_t BinaryFile::ReadLongwordUnsigned(Endianness endian) {
    uint16_t loWord, hiWord;
    
    if (endian == Endianness::Little) {
        loWord = ReadWordUnsigned(endian);
        hiWord = ReadWordUnsigned(endian);
    } else {
        hiWord = ReadWordUnsigned(endian);
        loWord = ReadWordUnsigned(endian);
    }
    
    return static_cast<uint32_t>((hiWord << 16) | loWord);
}

void BinaryFile::WriteLongwordSigned(int32_t value, Endianness endian) {
    int16_t hiWord = value >> 16;
    int16_t loWord = value & 0xFFFF;
    
    if (endian == Endianness::Little) {
        WriteWordSigned(loWord, endian);
        WriteWordSigned(hiWord, endian);
    } else {
        WriteWordSigned(hiWord, endian);
        WriteWordSigned(loWord, endian);
    }
}

void BinaryFile::WriteLongwordUnsigned(uint32_t value, Endianness endian) {
    uint16_t hiWord = value >> 16;
    uint16_t loWord = value & 0xFFFF;
    
    if (endian == Endianness::Little) {
        WriteWordUnsigned(loWord, endian);
        WriteWordUnsigned(hiWord, endian);
    } else {
        WriteWordUnsigned(hiWord, endian);
        WriteWordUnsigned(loWord, endian);
    }
}

std::string BinaryFile::ReadString(size_t length) {
    std::vector<uint8_t> bytes = ReadBytes(length);
    std::string result;
    result.reserve(length);
    
    for (uint8_t byte : bytes) {
        result += static_cast<char>(byte);
    }
    
    return result;
}

void BinaryFile::WriteString(const std::string& value) {
    for (char c : value) {
        WriteByteUnsigned(static_cast<uint8_t>(c));
    }
}

std::vector<uint8_t> BinaryFile::ReadBytes(size_t count) {
    std::vector<uint8_t> bytes(count);
    file.read(reinterpret_cast<char*>(bytes.data()), count);
    CheckEOF();
    return bytes;
}

void BinaryFile::WriteBytes(const std::vector<uint8_t>& data) {
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void BinaryFile::SwapWord(int16_t& word) {
    uint8_t hiByte = word >> 8;
    uint8_t loByte = word & 0xFF;
    word = static_cast<int16_t>((loByte << 8) | hiByte);
}

void BinaryFile::SwapLongword(int32_t& longword) {
    int16_t hiWord = longword >> 16;
    int16_t loWord = longword & 0xFFFF;
    SwapWord(hiWord);
    SwapWord(loWord);
    longword = static_cast<int32_t>((loWord << 16) | hiWord);
}

void BinaryFile::Nibbler(uint16_t value, uint8_t& byte1, uint8_t& byte2) {
    std::stringstream ss;
    ss << std::hex << std::setw(4) << std::setfill('0') << value;
    std::string hexVal = ss.str();
    
    if (hexVal.length() >= 2) {
        byte1 = std::stoi(hexVal.substr(0, 1), nullptr, 16);
        byte2 = std::stoi(hexVal.substr(1, 1), nullptr, 16);
    } else {
        byte1 = byte2 = 0;
    }
}

int16_t BinaryFile::ReadShort(uint8_t byte1, uint8_t byte2, Endianness endian) {
    uint8_t loByte, hiByte;
    
    if (endian == Endianness::Little) {
        loByte = byte1;
        hiByte = byte2;
    } else {
        hiByte = byte1;
        loByte = byte2;
    }
    
    return static_cast<int16_t>((hiByte << 8) | loByte);
}

void BinaryFile::CheckEOF() {
    if (file.eof()) {
        throw std::runtime_error("BinaryFile: Input past end of file.");
    }
}

uint16_t BinaryFile::SwapBytes(uint16_t value) {
    return ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
}

uint32_t BinaryFile::SwapBytes(uint32_t value) {
    return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | 
           ((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24);
} 