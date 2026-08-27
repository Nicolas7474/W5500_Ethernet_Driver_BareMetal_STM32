#pragma once

#include <cstdint>

namespace W5500_Reg
{
    // =========================================================
    // SPI Block Select
    // =========================================================

    namespace Block
    {
        constexpr uint8_t COMMON = 0x00; // Common registers

        constexpr uint8_t S0_REG = 0x01; // Socket 0 registers
        constexpr uint8_t S0_TX  = 0x02; // Socket 0 TX memory
        constexpr uint8_t S0_RX  = 0x03; // Socket 0 RX memory

        constexpr uint8_t S1_REG = 0x05; // Socket 1 registers
        constexpr uint8_t S1_TX  = 0x06; // Socket 1 TX memory
        constexpr uint8_t S1_RX  = 0x07; // Socket 1 RX memory

        constexpr uint8_t S2_REG = 0x09; // Socket 2 registers
        constexpr uint8_t S2_TX  = 0x0A; // Socket 2 TX memory
        constexpr uint8_t S2_RX  = 0x0B; // Socket 2 RX memory

        constexpr uint8_t S3_REG = 0x0D; // Socket 3 registers
        constexpr uint8_t S3_TX  = 0x0E; // Socket 3 TX memory
        constexpr uint8_t S3_RX  = 0x0F; // Socket 3 RX memory

        constexpr uint8_t S4_REG = 0x11; // Socket 4 registers
        constexpr uint8_t S4_TX  = 0x12; // Socket 4 TX memory
        constexpr uint8_t S4_RX  = 0x13; // Socket 4 RX memory

        constexpr uint8_t S5_REG = 0x15; // Socket 5 registers
        constexpr uint8_t S5_TX  = 0x16; // Socket 5 TX memory
        constexpr uint8_t S5_RX  = 0x17; // Socket 5 RX memory

        constexpr uint8_t S6_REG = 0x19; // Socket 6 registers
        constexpr uint8_t S6_TX  = 0x1A; // Socket 6 TX memory
        constexpr uint8_t S6_RX  = 0x1B; // Socket 6 RX memory

        constexpr uint8_t S7_REG = 0x1D; // Socket 7 registers
        constexpr uint8_t S7_TX  = 0x1E; // Socket 7 TX memory
        constexpr uint8_t S7_RX  = 0x1F; // Socket 7 RX memory
    }


    // =========================================================
    // Common Registers
    // =========================================================

    constexpr uint16_t MR        = 0x0000; // Mode register

    constexpr uint16_t GAR0      = 0x0001; // Gateway IP byte 0
    constexpr uint16_t GAR1      = 0x0002; // Gateway IP byte 1
    constexpr uint16_t GAR2      = 0x0003; // Gateway IP byte 2
    constexpr uint16_t GAR3      = 0x0004; // Gateway IP byte 3

    constexpr uint16_t SUBR0     = 0x0005; // Subnet mask byte 0
    constexpr uint16_t SUBR1     = 0x0006; // Subnet mask byte 1
    constexpr uint16_t SUBR2     = 0x0007; // Subnet mask byte 2
    constexpr uint16_t SUBR3     = 0x0008; // Subnet mask byte 3

    constexpr uint16_t SHAR0     = 0x0009; // MAC address byte 0
    constexpr uint16_t SHAR1     = 0x000A; // MAC address byte 1
    constexpr uint16_t SHAR2     = 0x000B; // MAC address byte 2
    constexpr uint16_t SHAR3     = 0x000C; // MAC address byte 3
    constexpr uint16_t SHAR4     = 0x000D; // MAC address byte 4
    constexpr uint16_t SHAR5     = 0x000E; // MAC address byte 5

    constexpr uint16_t SIPR0     = 0x000F; // IP address byte 0
    constexpr uint16_t SIPR1     = 0x0010; // IP address byte 1
    constexpr uint16_t SIPR2     = 0x0011; // IP address byte 2
    constexpr uint16_t SIPR3     = 0x0012; // IP address byte 3

    constexpr uint16_t INTLEVEL0 = 0x0013; // Interrupt level byte 0
    constexpr uint16_t INTLEVEL1 = 0x0014; // Interrupt level byte 1

    constexpr uint16_t IR        = 0x0015; // Interrupt register
    constexpr uint16_t IMR       = 0x0016; // Interrupt mask register
    constexpr uint16_t SIR       = 0x0017; // Socket interrupt register
    constexpr uint16_t SIMR      = 0x0018; // Socket interrupt mask

    constexpr uint16_t RTR0      = 0x0019; // Retry time byte 0
    constexpr uint16_t RTR1      = 0x001A; // Retry time byte 1
    constexpr uint16_t RCR       = 0x001B; // Retry count

    constexpr uint16_t PTIMER    = 0x001C; // PPPoE LCP timer
    constexpr uint16_t PMAGIC    = 0x001D; // PPPoE LCP magic number

    constexpr uint16_t PHAR0     = 0x001E; // PPPoE destination MAC byte 0
    constexpr uint16_t PHAR1     = 0x001F; // PPPoE destination MAC byte 1
    constexpr uint16_t PHAR2     = 0x0020; // PPPoE destination MAC byte 2
    constexpr uint16_t PHAR3     = 0x0021; // PPPoE destination MAC byte 3
    constexpr uint16_t PHAR4     = 0x0022; // PPPoE destination MAC byte 4
    constexpr uint16_t PHAR5     = 0x0023; // PPPoE destination MAC byte 5

    constexpr uint16_t PSID0     = 0x0024; // PPPoE session ID byte 0
    constexpr uint16_t PSID1     = 0x0025; // PPPoE session ID byte 1

    constexpr uint16_t PMRU0     = 0x0026; // PPPoE MTU byte 0
    constexpr uint16_t PMRU1     = 0x0027; // PPPoE MTU byte 1

    constexpr uint16_t UIPR0     = 0x0028; // Unreachable IP byte 0
    constexpr uint16_t UIPR1     = 0x0029; // Unreachable IP byte 1
    constexpr uint16_t UIPR2     = 0x002A; // Unreachable IP byte 2
    constexpr uint16_t UIPR3     = 0x002B; // Unreachable IP byte 3

    constexpr uint16_t UPORTR0   = 0x002C; // Unreachable port byte 0
    constexpr uint16_t UPORTR1   = 0x002D; // Unreachable port byte 1

    constexpr uint16_t PHYCFGR   = 0x002E; // PHY configuration

    constexpr uint16_t VERSIONR  = 0x0039; // Chip version


    // =========================================================
    // Mode Register Bits
    // =========================================================

    namespace MR_Bits
    {
        constexpr uint8_t RST   = (1U << 7); // Software reset
        constexpr uint8_t WOL   = (1U << 5); // Wake on LAN
        constexpr uint8_t PB    = (1U << 4); // Ping block
        constexpr uint8_t PPPoE = (1U << 3); // PPPoE mode
        constexpr uint8_t FARP  = (1U << 0); // Force ARP
    }


    // =========================================================
    // Common Interrupt Register Bits
    // =========================================================

    namespace IR_Bits
    {
        constexpr uint8_t CONFLICT = (1U << 7); // IP conflict
        constexpr uint8_t UNREACH  = (1U << 6); // Destination unreachable
        constexpr uint8_t PPPoE    = (1U << 5); // PPPoE connection
        constexpr uint8_t MP       = (1U << 4); // Magic packet
    }


    // =========================================================
    // PHY Configuration Register Bits
    // =========================================================

    namespace PHYCFGR_Bits
    {
        constexpr uint8_t RST    = (1U << 7); // PHY reset
        constexpr uint8_t OPMDC2 = (1U << 6); // Operation mode bit 2
        constexpr uint8_t OPMDC1 = (1U << 5); // Operation mode bit 1
        constexpr uint8_t OPMDC0 = (1U << 4); // Operation mode bit 0
        constexpr uint8_t OPMD   = (1U << 3); // Operation mode
        constexpr uint8_t DPX    = (1U << 2); // Duplex status
        constexpr uint8_t LNK    = (1U << 0); // Link status
    }


    // =========================================================
    // Socket Registers
    // =========================================================

    constexpr uint16_t Sn_MR         = 0x0000; // Socket mode
    constexpr uint16_t Sn_CR         = 0x0001; // Socket command
    constexpr uint16_t Sn_IR         = 0x0002; // Socket interrupt
    constexpr uint16_t Sn_SR         = 0x0003; // Socket status

    constexpr uint16_t Sn_PORT0      = 0x0004; // Source port byte 0 (stores the MSB of the port)
    constexpr uint16_t Sn_PORT1      = 0x0005; // Source port byte 1 (stores the LSB of the port)

    constexpr uint16_t Sn_DHAR0      = 0x0006; // Destination MAC byte 0
    constexpr uint16_t Sn_DHAR1      = 0x0007; // Destination MAC byte 1
    constexpr uint16_t Sn_DHAR2      = 0x0008; // Destination MAC byte 2
    constexpr uint16_t Sn_DHAR3      = 0x0009; // Destination MAC byte 3
    constexpr uint16_t Sn_DHAR4      = 0x000A; // Destination MAC byte 4
    constexpr uint16_t Sn_DHAR5      = 0x000B; // Destination MAC byte 5

    constexpr uint16_t Sn_DIPR0      = 0x000C; // Destination IP byte 0
    constexpr uint16_t Sn_DIPR1      = 0x000D; // Destination IP byte 1
    constexpr uint16_t Sn_DIPR2      = 0x000E; // Destination IP byte 2
    constexpr uint16_t Sn_DIPR3      = 0x000F; // Destination IP byte 3

    constexpr uint16_t Sn_DPORT0     = 0x0010; // Destination port byte 0
    constexpr uint16_t Sn_DPORT1     = 0x0011; // Destination port byte 1

    constexpr uint16_t Sn_MSSR0      = 0x0012; // Maximum segment size byte 0
    constexpr uint16_t Sn_MSSR1      = 0x0013; // Maximum segment size byte 1

    constexpr uint16_t Sn_TOS        = 0x0015; // IP TOS
    constexpr uint16_t Sn_TTL        = 0x0016; // IP TTL

    constexpr uint16_t Sn_RXBUF_SIZE = 0x001E; // RX buffer size
    constexpr uint16_t Sn_TXBUF_SIZE = 0x001F; // TX buffer size

    constexpr uint16_t Sn_TX_FSR0    = 0x0020; // TX free size byte 0
    constexpr uint16_t Sn_TX_FSR1    = 0x0021; // TX free size byte 1

    constexpr uint16_t Sn_TX_RD0     = 0x0022; // TX read pointer byte 0
    constexpr uint16_t Sn_TX_RD1     = 0x0023; // TX read pointer byte 1

    constexpr uint16_t Sn_TX_WR0     = 0x0024; // TX write pointer byte 0
    constexpr uint16_t Sn_TX_WR1     = 0x0025; // TX write pointer byte 1

    constexpr uint16_t Sn_RX_RSR0    = 0x0026; // RX received size byte 0
    constexpr uint16_t Sn_RX_RSR1    = 0x0027; // RX received size byte 1

    constexpr uint16_t Sn_RX_RD0     = 0x0028; // RX read pointer byte 0
    constexpr uint16_t Sn_RX_RD1     = 0x0029; // RX read pointer byte 1

    constexpr uint16_t Sn_RX_WR0     = 0x002A; // RX write pointer byte 0
    constexpr uint16_t Sn_RX_WR1     = 0x002B; // RX write pointer byte 1

    constexpr uint16_t Sn_IMR        = 0x002C; // Socket interrupt mask
    constexpr uint16_t Sn_FRAG0      = 0x002D; // IP fragment byte 0
    constexpr uint16_t Sn_FRAG1      = 0x002E; // IP fragment byte 1
    constexpr uint16_t Sn_KPALVTR    = 0x002F; // Keep-alive timer


    // =========================================================
    // Socket Mode Register
    // =========================================================

    namespace Sn_MR_Bits
    {
        constexpr uint8_t MULTI = (1U << 7); // Multicast
        constexpr uint8_t MF    = (1U << 6); // MAC filter
        constexpr uint8_t ND    = (1U << 5); // No delayed ACK

        constexpr uint8_t CLOSE  = 0x00; // Closed
        constexpr uint8_t TCP    = 0x01; // TCP
        constexpr uint8_t UDP    = 0x02; // UDP
        constexpr uint8_t IPRAW  = 0x03; // IPRAW
        constexpr uint8_t MACRAW = 0x04; // MACRAW
    }


    // =========================================================
    // Socket Command Register
    // =========================================================

    namespace Sn_CR_Command
    {
        constexpr uint8_t OPEN      = 0x01; // Open
        constexpr uint8_t LISTEN    = 0x02; // Listen
        constexpr uint8_t CONNECT   = 0x04; // Connect
        constexpr uint8_t DISCON    = 0x08; // Disconnect
        constexpr uint8_t CLOSE     = 0x10; // Close
        constexpr uint8_t SEND      = 0x20; // Send
        constexpr uint8_t SEND_MAC  = 0x21; // Send MAC
        constexpr uint8_t SEND_KEEP = 0x22; // Send keep-alive
        constexpr uint8_t RECV      = 0x40; // Receive
    }


    // =========================================================
    // Socket Interrupt Register Bits
    // =========================================================

    namespace Sn_IR_Bits
    {
        constexpr uint8_t CON     = (1U << 0); // Connection
        constexpr uint8_t DISCON  = (1U << 1); // Disconnect
        constexpr uint8_t RECV    = (1U << 2); // Receive
        constexpr uint8_t TIMEOUT = (1U << 3); // Timeout
        constexpr uint8_t SENDOK  = (1U << 4); // Send complete
    }


    // =========================================================
    // Socket Status Values
    // =========================================================

    namespace Sn_SR_Status
    {
        constexpr uint8_t CLOSED      = 0x00; // Closed
        constexpr uint8_t INIT        = 0x13; // TCP initialized
        constexpr uint8_t LISTEN      = 0x14; // Listening
        constexpr uint8_t SYNSENT     = 0x15; // SYN sent
        constexpr uint8_t SYNRECV     = 0x16; // SYN received
        constexpr uint8_t ESTABLISHED = 0x17; // Connected
        constexpr uint8_t FIN_WAIT    = 0x18; // FIN wait
        constexpr uint8_t CLOSING     = 0x1A; // Closing
        constexpr uint8_t TIME_WAIT   = 0x1B; // Time wait
        constexpr uint8_t CLOSE_WAIT  = 0x1C; // Close wait
        constexpr uint8_t LAST_ACK    = 0x1D; // Last ACK

        constexpr uint8_t UDP         = 0x22; // UDP
        constexpr uint8_t IPRAW       = 0x32; // IPRAW
        constexpr uint8_t MACRAW      = 0x42; // MACRAW
    }


    // =========================================================
    // Socket Buffer Size Values
    // =========================================================

    namespace BufferSize
    {
        constexpr uint8_t DISABLED = 0;  // 0 KB
        constexpr uint8_t KB_1     = 1;  // 1 KB
        constexpr uint8_t KB_2     = 2;  // 2 KB
        constexpr uint8_t KB_4     = 4;  // 4 KB
        constexpr uint8_t KB_8     = 8;  // 8 KB
        constexpr uint8_t KB_16    = 16; // 16 KB
    }


    // =========================================================
    // Socket Block Helpers
    // =========================================================

    constexpr uint8_t SocketRegBlock(uint8_t socket)
    {
        return static_cast<uint8_t>(1U + socket * 4U);
    }

    constexpr uint8_t SocketTxBlock(uint8_t socket)
    {
        return static_cast<uint8_t>(2U + socket * 4U);
    }

    constexpr uint8_t SocketRxBlock(uint8_t socket)
    {
        return static_cast<uint8_t>(3U + socket * 4U);
    }
}
