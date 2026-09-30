#pragma once

#include <cstddef>
#include <cstdint>

enum class DemoMessageKind
{
    NetworkMessage,
    ClientArchive,
    Unknown
};

struct DemoMessage
{
    std::size_t fileOffset = 0;

    uint8_t type = 0;
    uint32_t sequence = 0;

    // Used by network messages.
    uint32_t size = 0;

    std::size_t dataOffset = 0;
    std::size_t dataSize = 0;
    std::size_t nextOffset = 0;

    DemoMessageKind kind = DemoMessageKind::Unknown;
};