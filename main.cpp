#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "DemoReader.h"
#include "DemoMessage.h"
#include "Huffman.h"

constexpr std::size_t NETCHAN_MAXBUFFER_SIZE =
    0x20000 * 10;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage:\n";
        std::cout << "  cod5demotool <demo.dm_6>\n";
        return 1;
    }

    DemoReader demo(argv[1]);

    if (!demo.open())
        return 1;

    std::cout << "CoD WaW Huffman Test\n";
    std::cout << "====================\n\n";

    DemoMessage message;

    while (demo.readNextMessage(message))
    {
        if (message.kind != DemoMessageKind::NetworkMessage)
            continue;

        if (message.size <= 4)
            continue;

        std::vector<uint8_t> payload;

        if (!demo.readBytesAt(
                message.dataOffset,
                message.size,
                payload))
        {
            std::cerr << "Could not read payload.\n";
            return 1;
        }

        const uint32_t dummy =
            static_cast<uint32_t>(payload[0]) |
            (static_cast<uint32_t>(payload[1]) << 8) |
            (static_cast<uint32_t>(payload[2]) << 16) |
            (static_cast<uint32_t>(payload[3]) << 24);

        const int compressedSize =
            static_cast<int>(payload.size() - 4);

        std::vector<uint8_t> output(
            NETCHAN_MAXBUFFER_SIZE
        );

        std::cout << "Network message\n";
        std::cout << "---------------\n";
        std::cout << "Sequence:        "
                  << message.sequence << '\n';

        std::cout << "Dummy:           "
                  << dummy << '\n';

        std::cout << "Compressed size: "
                  << compressedSize << '\n';

        // Build the same Huffman tree used by
        // the CoD4 reference implementation.
        CoD4::DM1::Huffman::InitMain();

        const int decompressedSize =
            CoD4::DM1::Huffman::Decompress(
                payload.data() + 4,
                compressedSize,
                output.data(),
                static_cast<int>(output.size())
            );

        std::cout << "Decompressed:    "
                  << decompressedSize
                  << " bytes\n\n";

        std::cout
            << "First decompressed bytes:\n";

        const int bytesToShow =
            std::min(decompressedSize, 128);

        for (int i = 0; i < bytesToShow; ++i)
        {
            if (i % 16 == 0)
            {
                std::cout
                    << std::setw(8)
                    << std::setfill('0')
                    << std::hex
                    << i
                    << "  ";
            }

            std::cout
                << std::setw(2)
                << std::setfill('0')
                << static_cast<int>(output[i])
                << ' ';

            if ((i + 1) % 16 == 0)
                std::cout << '\n';
        }

        if (bytesToShow % 16 != 0)
            std::cout << '\n';

        std::cout << std::dec;

        std::cout
            << "\nFirst byte as svc command: "
            << static_cast<int>(output[0])
            << '\n';

        break;
    }

    demo.close();

    return 0;
}