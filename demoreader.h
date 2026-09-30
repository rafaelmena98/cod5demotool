#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "DemoMessage.h"

class DemoReader
{
public:
    explicit DemoReader(const std::string& filePath);

    bool open();
    void close();

    bool isOpen() const;
    std::size_t getFileSize() const;

    bool readBytes(
        std::size_t count,
        std::vector<uint8_t>& output
    );

    bool readBytesAt(
        std::size_t offset,
        std::size_t count,
        std::vector<uint8_t>& output
    );

    uint8_t readUInt8();
    uint16_t readUInt16();
    uint32_t readUInt32();
    int32_t readInt32();

    std::size_t position();
    bool seek(std::size_t offset);

    bool readNextMessage(DemoMessage& message);

private:
    std::string filePath;
    std::ifstream file;
    std::size_t fileSize = 0;
};