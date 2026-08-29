#pragma once

#include <cstdint>
#include <span>
#include "spi.hpp"

#define ASSERT_OK(expr) if(static_cast<BareM_Status>(expr) != BareM_Status::OK) { GPIOE->ODR ^= (1U << 2); for (;;); }

/* This class uses a static-only design pattern (often used for hardware abstraction layers, drivers, or singletons)
Because all member functions and variables are declared as static, you do not need to instantiate an object. 
*/
class W5500
{
public:

    // ---------- Types ----------

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

    static BareM_Status GetMacAddress(std::span<uint8_t, 6> mac);
    static BareM_Status GetIPAddress(std::span<uint8_t, 4> ip);
    static BareM_Status GetSubnetMask(std::span<uint8_t, 4> mask);
    static BareM_Status GetGateway(std::span<uint8_t, 4> gateway);


    /* ---------- Socket buffer configuration ---------- */

    // Default: 2 KB TX + 2 KB RX
    static BareM_Status SetSocketBufferSize(uint8_t socket,
            BufferSize txSize = BufferSize::KB2, BufferSize rxSize = BufferSize::KB2);


    /* ---------- Socket management ---------- */

    static BareM_Status SocketOpen(uint8_t socket, Protocol protocol, uint16_t port);
    static BareM_Status SocketClose(uint8_t socket);

    static BareM_Status SocketGetDestination(
            uint8_t socket, std::span<uint8_t, 4> ip, uint16_t& port);

    static BareM_Status SocketSetDestination(
            uint8_t socket, std::span<const uint8_t, 4> ip, uint16_t port);


    /* ---------- Socket communication ---------- */

    static BareM_Status SocketSend(uint8_t socket, std::span<const uint8_t> data);
    static BareM_Status SocketReceive(uint8_t socket, std::span<uint8_t> data);

    static BareM_Status SocketGetReceivedSize(uint8_t socket, uint16_t& size);
    static BareM_Status SocketGetTxFreeSize(uint8_t socket, uint16_t& size);

    static BareM_Status WaitForSendComplete(uint8_t socket);


    /* ---------- Socket interrupts ---------- */

    static BareM_Status SocketSetInterruptMask(uint8_t socket, uint8_t mask);
    static BareM_Status SetSocketInterruptMask(uint8_t mask);

    static BareM_Status GetSocketInterrupt(uint8_t socket, uint8_t& interrupt);
    static BareM_Status ClearSocketInterrupt(uint8_t socket, uint8_t mask);


    /* ---------- Diagnostic ---------- */

    static BareM_Status ReadVersion(uint8_t& version);
    static BareM_Status ReadPhyConfig(uint8_t& phyConfig);
    static BareM_Status SocketGetStatus(uint8_t socket, uint8_t& status);


    /* ---------- API Read / Write ---------- */

    static BareM_Status ProcessUdpReceive(uint8_t socket);
    static uint16_t ReadRx(uint8_t* dest, uint16_t maxLen);

    static BareM_Status ProcessInterrupt(uint8_t socket);
};
