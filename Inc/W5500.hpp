#pragma once

#include <cstdint>
#include <span>
#include "spi.hpp"


class W5500
{
public:

    enum class Protocol : uint8_t
    {
        TCP = 0x01, UDP = 0x02, IPRAW = 0x03, MACRAW = 0x04
    };

    enum class BufferSize : uint8_t
    {
        KB0 = 0, KB1 = 1, KB2 = 2, KB4 = 4, KB8 = 8, KB16 = 16
    };

   /* ---------- Initialization ---------- */

    static BareM_Status Init();


  /* ---------- Network configuration ---------- */

    static BareM_Status SetMacAddress(std::span<const uint8_t, 6> mac);
    static BareM_Status SetIPAddress(std::span<const uint8_t, 4> ip);
    static BareM_Status SetSubnetMask(std::span<const uint8_t, 4> mask);
    static BareM_Status SetGateway(std::span<const uint8_t, 4> gateway);


    /* ---------- Socket buffer configuration ---------- */

    // Default: 2 KB TX + 2 KB RX
    static BareM_Status SetSocketBufferSize(uint8_t socket,
    		BufferSize txSize = BufferSize::KB2, BufferSize rxSize = BufferSize::KB2);


    /* --------- Diagnostic ---------- */

    static BareM_Status ReadVersion(uint8_t& version);


    /* ---------- Socket ---------- */

    static BareM_Status SocketOpen(uint8_t socket, Protocol protocol, uint16_t port);
    static BareM_Status SocketClose(uint8_t socket);
    static BareM_Status SocketSend(uint8_t socket, std::span<const uint8_t> data);
    static BareM_Status SocketReceive(uint8_t socket, std::span<uint8_t> data);
};
