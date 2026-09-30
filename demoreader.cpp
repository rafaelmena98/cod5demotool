#include "DemoReader.h"

#include <iostream>
#include <stdexcept>

DemoReader::DemoReader(const std::string& filePath)
    : filePath(filePath)
{
}

bool DemoReader::open()
{
    file.open(filePath, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Could not open demo: "
                  << filePath << '\n';
        return false;
    }

    file.seekg(0, std::ios::end);
    fileSize = static_cast<std::size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    return true;
}

void DemoReader::close()
{
    if (file.is_open())
        file.close();
}

bool DemoReader::isOpen() const
{
    return file.is_open();
}

std::size_t DemoReader::getFileSize() const
{
    return fileSize;
}

bool DemoReader::readBytes(
    std::size_t count,
    std::vector<uint8_t>& output)
{
    output.resize(count);

    file.read(
        reinterpret_cast<char*>(output.data()),
        static_cast<std::streamsize>(count)
    );

    return static_cast<bool>(file);
}

uint8_t DemoReader::readUInt8()
{
    uint8_t value = 0;

    file.read(
        reinterpret_cast<char*>(&value),
        sizeof(value)
    );

    if (!file)
        throw std::runtime_error(
            "Failed to read uint8"
        );

    return value;
}

uint16_t DemoReader::readUInt16()
{
    uint16_t value = 0;

    file.read(
        reinterpret_cast<char*>(&value),
        sizeof(value)
    );

    if (!file)
        throw std::runtime_error(
            "Failed to read uint16"
        );

    return value;
}

uint32_t DemoReader::readUInt32()
{
    uint32_t value = 0;

    file.read(
        reinterpret_cast<char*>(&value),
        sizeof(value)
    );

    if (!file)
        throw std::runtime_error(
            "Failed to read uint32"
        );

    return value;
}

int32_t DemoReader::readInt32()
{
    int32_t value = 0;

    file.read(
        reinterpret_cast<char*>(&value),
        sizeof(value)
    );

    if (!file)
        throw std::runtime_error(
            "Failed to read int32"
        );

    return value;
}

std::size_t DemoReader::position()
{
    std::streampos pos = file.tellg();

    if (pos == std::streampos(-1))
        return fileSize;

    return static_cast<std::size_t>(pos);
}

bool DemoReader::seek(std::size_t offset)
{
    if (offset > fileSize)
        return false;

    file.clear();

    file.seekg(
        static_cast<std::streamoff>(offset),
        std::ios::beg
    );

    return static_cast<bool>(file);
}

bool DemoReader::readBytesAt(
    std::size_t offset,
    std::size_t count,
    std::vector<uint8_t>& output)
{
    if (!file.is_open())
        return false;

    if (offset > fileSize ||
        count > fileSize - offset)
    {
        return false;
    }

    const std::size_t oldPosition = position();

    if (!seek(offset))
        return false;

    const bool result = readBytes(count, output);

    if (!seek(oldPosition))
        return false;

    return result;
}

bool DemoReader::readNextMessage(
    DemoMessage& message)
{
    if (!file.is_open())
        return false;

    const std::size_t startOffset = position();

    if (startOffset >= fileSize)
        return false;

    message = DemoMessage{};
    message.fileOffset = startOffset;

    try
    {
        message.type = readUInt8();

        switch (message.type)
        {
            // Network message
            case 0:
            {
                message.kind =
                    DemoMessageKind::NetworkMessage;

                // Need sequence + size.
                if (position() + 8 > fileSize)
                    return false;

                message.sequence = readUInt32();
                message.size = readUInt32();

                // End-of-demo marker.
                // -1 stored as uint32_t becomes 0xFFFFFFFF.
                if (message.sequence == 0xFFFFFFFF &&
                    message.size == 0xFFFFFFFF)
                {
                    message.dataOffset = position();
                    message.dataSize = 0;
                    message.nextOffset = position();

                    std::cout
                        << "\nEnd of demo reached\n";

                    return false;
                }

                message.dataOffset = position();
                message.dataSize = message.size;

                const std::size_t remaining =
                    fileSize - message.dataOffset;

                if (message.dataSize > remaining)
                {
                    std::cerr
                        << "\nInvalid Type 0 message\n";

                    std::cerr
                        << "  Offset:    "
                        << message.fileOffset << '\n';

                    std::cerr
                        << "  Sequence:  "
                        << message.sequence << '\n';

                    std::cerr
                        << "  Size:      "
                        << message.size << '\n';

                    std::cerr
                        << "  Remaining: "
                        << remaining << '\n';

                    return false;
                }

                message.nextOffset =
                    message.dataOffset +
                    message.dataSize;

                return seek(message.nextOffset);
            }

            // Client archive
            case 1:
            {
                message.kind =
                    DemoMessageKind::ClientArchive;

                // Type has already been read.
                // Client archive:
                // 4 byte sequence + 48 byte archive.
                constexpr std::size_t archiveDataSize = 48;

                if (position() + 4 + archiveDataSize >
                    fileSize)
                {
                    std::cerr
                        << "\nIncomplete Type 1 archive\n";

                    return false;
                }

                message.sequence = readUInt32();

                message.dataOffset = position();
                message.dataSize = archiveDataSize;

                message.nextOffset =
                    message.dataOffset +
                    message.dataSize;

                return seek(message.nextOffset);
            }

            default:
            {
                message.kind =
                    DemoMessageKind::Unknown;

                message.dataOffset = position();
                message.nextOffset = position();

                std::cerr
                    << "\nUnknown demo record type\n";

                std::cerr
                    << "  Offset: "
                    << message.fileOffset << '\n';

                std::cerr
                    << "  Type:   "
                    << static_cast<int>(message.type)
                    << '\n';

                return false;
            }
        }
    }
    catch (const std::runtime_error& error)
    {
        std::cerr
            << "\nRead error at offset "
            << startOffset << '\n';

        std::cerr
            << error.what() << '\n';

        return false;
    }
}