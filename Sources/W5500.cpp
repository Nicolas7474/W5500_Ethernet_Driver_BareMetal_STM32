/*	The current W5500 API uses static member functions,
 * therefore main.c can call W5500::Init() directly and doesn't need to instantiate a W5500 object. 	*/

#include "W5500.hpp"
#include "myConfig.h"

#include <algorithm>
#include "W5500_registers.hpp"


extern SpiDriver spi3;


namespace
{
	/* File-local implementation functions: An unnamed namespace is a modern C++ way of getting internal/file-local linkage
	without having to put static in front of every helper. Nobody outside this file can use these names. */

    constexpr uint8_t MAX_SOCKETS = 8;

    constexpr uint32_t W5500_RESET_TIME_MS = 50;
    constexpr uint32_t SPI_WAIT_TIMEOUT_MS  = 100;
    constexpr uint32_t SOCKET_CMD_TIMEOUT_MS = 100;

    // PD0 = W5500 CS
    constexpr uint32_t CS_LOW  = GPIO_BSRR_BR0;
    constexpr uint32_t CS_HIGH = GPIO_BSRR_BS0;

    // PD1 = W5500 RESET
    constexpr uint32_t RESET_HIGH = GPIO_BSRR_BS1;

    // ---------------------------------------------------------
    // Default socket buffer allocation
    // ---------------------------------------------------------

    W5500::BufferSize txBufferSize[MAX_SOCKETS]
       {
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2
       };

       W5500::BufferSize rxBufferSize[MAX_SOCKETS]
       {
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2,
           W5500::BufferSize::KB2
       };

    // =========================================================
    // Chip Select
    // =========================================================

    inline void CS_Low()
    {
        GPIOD->BSRR = CS_LOW;
    }

    inline void CS_High()
    {
        GPIOD->BSRR = CS_HIGH;
    }

    // =========================================================
    // W5500 SPI control byte
    //
    // BSB[7:3] | R/W | OM[1:0]
    //
    // OM = 00 -> Variable length data mode
    // =========================================================

    constexpr uint8_t MakeControlByte(uint8_t block, bool write)
    {
        return static_cast<uint8_t>((block << 3) | (write ? 0x04U : 0x00U)); // 0x04U = RWB on bit 2
    }

    // =========================================================
    // Buffer size conversion
    // =========================================================

    constexpr uint16_t BufferBytes(W5500::BufferSize size)
    {
        return static_cast<uint16_t>(static_cast<uint8_t>(size) * 1024U);
    }

    // =========================================================
    // Validate socket number
    // =========================================================

    constexpr bool ValidSocket(uint8_t socket)
    {
        return socket < MAX_SOCKETS;
    }

    BareM_Status WaitForSpiReady(uint32_t timeoutMs)
    {
        const uint32_t timeout = GetSysTick() + timeoutMs;

        while (spi3.GetState() != SpiState::READY)
        {
            if (GetSysTick() > timeout)
                return BareM_Status::TIMEOUT;
        }

        return BareM_Status::OK;
    }

    // =========================================================
    // Generic WRITE
    //
    // Header = polling
    // Payload = DMA
    //
    // CS remains LOW for the complete transaction.
    // =========================================================

    BareM_Status Write(uint16_t address, uint8_t block, std::span<const uint8_t> data)
    {
        if (data.empty())
        	return BareM_Status::ERROR;

        const uint8_t header[3] { static_cast<uint8_t>(address >> 8), static_cast<uint8_t>(address), MakeControlByte(block, true) };

        CS_Low();
        // Small header: polling avoids SPI RX overrun.
        auto status = spi3.Transmit(std::span<const uint8_t>(header), 10);

        if (status != BareM_Status::OK)
        {
            CS_High();
            return status;
        }

        if (data.size() < 8)
        {
        	// Small payload: polling is faster if byte length < 20
        	// Nonetheless we choose 8 bytes max to avoid blocking the CPU for too long
        	status = spi3.Transmit(data, 10);
        }
        else
        {
        	// DMA: slightly higher latency for small transfers up to 20 bytes,
        	// but significantly reduces CPU occupation.
        	status = spi3.Transmit_DMA(data);

        	if (status == BareM_Status::OK)
        		status = WaitForSpiReady(SPI_WAIT_TIMEOUT_MS); // only for DMA
        }
        CS_High();

        return status;
    }

    // =========================================================
    // Generic READ    
    // CS remains LOW for the complete transaction.
    // =========================================================

    BareM_Status Read(uint16_t address, uint8_t block, std::span<uint8_t> data)
    {
        if (data.empty())
            return BareM_Status::ERROR;

        const uint8_t header[3] { static_cast<uint8_t>(address >> 8), static_cast<uint8_t>(address), MakeControlByte(block, false) };

        CS_Low();

        // Small header: polling drains the dummy RX bytes.
        auto status = spi3.Transmit(std::span<const uint8_t>(header), 10);

        if (status != BareM_Status::OK) {
            CS_High();
            return status;
        }

        if (data.size() < 8) {
            // Small payload: polling is faster for short lengths (avoids DMA setup overhead)
            status = spi3.Receive(data, 10);
        }
        else {
            // Payload: DMA generates the SPI clocks and receives data.
            status = spi3.Receive_DMA(data);

            if (status == BareM_Status::OK)
                status = WaitForSpiReady(SPI_WAIT_TIMEOUT_MS); // only for DMA
        }

        CS_High();

        return status;
    }


    // =========================================================
    // Small 8-bit READ
    //
    // Kept as polling because this is exactly the type of
    // small transaction for which DMA is unnecessary.
    // =========================================================

    BareM_Status Read8(uint16_t address, uint8_t block, uint8_t& value)
    {
        const uint8_t tx[4] { static_cast<uint8_t>(address >> 8), static_cast<uint8_t>(address), MakeControlByte(block, false), 0x00 };

        uint8_t rx[4]{};

        CS_Low();

        auto status = spi3.TransmitReceive(std::span<const uint8_t>(tx), std::span<uint8_t>(rx), 10);

        if (status != BareM_Status::OK)
        {
            CS_High();
            return status;
        }

        CS_High();

        value = rx[3]; // First three received bytes correspond to the header.

        return BareM_Status::OK;
    }


    // =========================================================
    // 8-bit WRITE
    // =========================================================

    BareM_Status Write8(uint16_t address, uint8_t block, uint8_t value)
    {
        return Write(address, block, std::span<const uint8_t>(&value, 1));
    }

    // =========================================================
    // 16-bit READ
    // =========================================================

    BareM_Status Read16(uint16_t address, uint8_t block, uint16_t& value)
    {
        uint8_t data[2]{};

        auto status = Read(address, block, std::span<uint8_t>(data));

        if (status != BareM_Status::OK)
            return status;

        value = static_cast<uint16_t>((static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]));

        return BareM_Status::OK;
    }


    // =========================================================
    // 16-bit WRITE
    // =========================================================

    BareM_Status Write16(uint16_t address, uint8_t block, uint16_t value)
    {
        const uint8_t data[2] { static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value) };

        return Write(address, block, std::span<const uint8_t>(data));
    }


    // =========================================================
    // Socket register helpers
    // 8-bit registers: Such as Socket Command (Sn_CR), Socket Interrupt (Sn_IR), and Status (Sn_SR)
    // 16-bit registers: Such as Source Port (Sn_PORT), Maximum Segment Size (Sn_MSS), or pointer/buffer registers
    // =========================================================

    BareM_Status SocketRead8(uint8_t socket, uint16_t address, uint8_t& value)
    {
        return Read8(address, W5500_Reg::SocketRegBlock(socket), value);
    }

    BareM_Status SocketWrite8(uint8_t socket, uint16_t address, uint8_t value)
    {
        return Write8(address, W5500_Reg::SocketRegBlock(socket), value);
    }

    BareM_Status SocketRead16(uint8_t socket, uint16_t address, uint16_t& value)
    {
        return Read16(address, W5500_Reg::SocketRegBlock(socket), value);
    }

    BareM_Status SocketWrite16(uint8_t socket, uint16_t address, uint16_t value)
    {
        return Write16(address, W5500_Reg::SocketRegBlock(socket), value);
    }


    // =========================================================
    // Stable Sn_TX_FSR read
    // The W5500's internal hardware updates the Sn_TX_FSR (TX Free Size) and Sn_RX_RSR (RX Received Size) 
    // registers dynamically as packets are sent or received. Because these are 16-bit values 
    // and the internal data bus might be updating the high byte and low byte sequentially, 
    // reading them at the exact moment the hardware is changing them can result in garbage/torn data.
    // This "read twice and compare" recommended strategy guarantees you have a perfectly stable, 
    // accurate read before deciding how much data to push into the TX buffer!
    // =========================================================

    BareM_Status GetStableTxFreeSize(uint8_t socket, uint16_t& size)
    {
        uint16_t value1;
        uint16_t value2;

        // Loop until we get two consecutive identical readings
        do
        {   
            // 1st snapshot of the TX Free Size Register
            auto status = SocketRead16(socket, W5500_Reg::Sn_TX_FSR0, value1);

            if (status != BareM_Status::OK)
                return status;

            // 2nd snapshot of the TX Free Size Register
            status = SocketRead16(socket, W5500_Reg::Sn_TX_FSR0, value2);

            if (status != BareM_Status::OK)
                return status;

        } while (value1 != value2); // If values mismatch, the W5500 was actively changing them. Try again.

        size = value1; // Pass the verified, stable value back to the caller

        return BareM_Status::OK;
    }


    // =========================================================
    // Stable Sn_RX_RSR read
    // =========================================================

    BareM_Status GetStableRxReceivedSize(uint8_t socket, uint16_t& size)
    {
        uint16_t value1;
        uint16_t value2;

        do
        {
            auto status = SocketRead16(socket, W5500_Reg::Sn_RX_RSR0, value1);

            if (status != BareM_Status::OK)
                return status;

            status = SocketRead16(socket, W5500_Reg::Sn_RX_RSR0, value2);

            if (status != BareM_Status::OK)
                return status;

        } while (value1 != value2);

        size = value1;

        return BareM_Status::OK;
    }


    // =========================================================
    // Socket command
    // =========================================================

    BareM_Status SocketCommand(uint8_t socket, uint8_t command)
    {
        // Issue the command by writing it to the Socket Command Register (Sn_CR)
        auto status = SocketWrite8(socket, W5500_Reg::Sn_CR, command);

        if (status != BareM_Status::OK)
            return status;

        uint8_t commandValue;
        const uint32_t timeout = GetSysTick() + SOCKET_CMD_TIMEOUT_MS;

        do      // Poll the Command Register (Sn_CR) until the W5500 clears the command bit back to 0
        {
            // Read back the current value of the Command Register for this socket
            status = SocketRead8(socket, W5500_Reg::Sn_CR, commandValue);

            if (status != BareM_Status::OK)
                return status;

            if (GetSysTick() > timeout)
                return BareM_Status::TIMEOUT;

        } while (commandValue != 0); // The W5500 automatically clears Sn_CR to 0x00 once the command finishes executing

        return BareM_Status::OK;
    }


    // =========================================================
    // Socket buffer configuration
    // =========================================================

    BareM_Status ConfigureSocketBuffer(uint8_t socket)
    {
        auto status = SocketWrite8(socket, W5500_Reg::Sn_TXBUF_SIZE, static_cast<uint8_t>(txBufferSize[socket]));

        if (status != BareM_Status::OK)
            return status;


        return SocketWrite8(socket, W5500_Reg::Sn_RXBUF_SIZE, static_cast<uint8_t>(rxBufferSize[socket]));
    }


    // =========================================================
    // Check total configured memory
    //
    // W5500 has 16 KB TX + 16 KB RX memory.
    // =========================================================

    bool ValidBufferAllocation()
    {
        uint16_t txTotal = 0;
        uint16_t rxTotal = 0;

        for (uint8_t socket = 0; socket < MAX_SOCKETS; ++socket)
        {
            txTotal += BufferBytes(txBufferSize[socket]);
            rxTotal += BufferBytes(rxBufferSize[socket]);
        }

        return
            (txTotal <= 16384U) &&
            (rxTotal <= 16384U);
    }
}						// --- End of namespace ---


// ============================================================================
// INITIALIZATION
// ============================================================================


BareM_Status W5500::Init()
{
  
    /* ----------- Reset (active low) at least 500 us ---------------------   */
    uint32_t start = GetSysTick();  // Spi3_LowLevelInit() deliberately leaves PD1 LOW
    while ((GetSysTick() - start) < W5500_RESET_TIME_MS);

    GPIOD->BSRR = RESET_HIGH; // Then release W5500 hardware reset
    // Wait for W5500 internal startup / PLL
    start = GetSysTick();
    while ((GetSysTick() - start) < W5500_RESET_TIME_MS);

    
    /* ----------- First hardware sanity check: VERSIONR ------------------- */
    uint8_t version = 0;
    // Offset Address for Chip version on Common Register = 0x0039
    auto status = ReadVersion(version);

    if (status != BareM_Status::OK)
        return status;

    // W5500 VERSIONR must return value = 0x04
    if (version != 0x04)
        return BareM_Status::ERROR;
  
    /* ----------- Wait for the PHY Link to be UP and perform check ------- */ 
    start = GetSysTick();
    while ((GetSysTick() - start) < 2800); // No SPI polling - BLOCKING        
   
    uint8_t phyConfig = 0;  // Check the actual hardware state once

    if (ReadPhyConfig(phyConfig) != BareM_Status::OK)
        return BareM_Status::ERROR;

    if ((phyConfig & W5500_Reg::PHYCFGR_Bits::LNK) == 0)
        return BareM_Status::TIMEOUT;

    /* ----------- Configure default 2 KB TX + 2 KB RX on every socket ---- */
    for (uint8_t socket = 0; socket < MAX_SOCKETS; ++socket) {
        status = ConfigureSocketBuffer(socket);

        if (status != BareM_Status::OK)
            return status;
    }

    return BareM_Status::OK;
}


// ============================================================================
// NETWORK CONFIGURATION
// ============================================================================

BareM_Status W5500::SetMacAddress(std::span<const uint8_t, 6> mac)
{
    return Write(W5500_Reg::SHAR0, W5500_Reg::Block::COMMON, mac);
}

BareM_Status W5500::SetIPAddress(std::span<const uint8_t, 4> ip)
{
    return Write(W5500_Reg::SIPR0, W5500_Reg::Block::COMMON, ip);
}

BareM_Status W5500::SetSubnetMask(std::span<const uint8_t, 4> mask)
{
    return Write(W5500_Reg::SUBR0, W5500_Reg::Block::COMMON, mask);
}

BareM_Status W5500::SetGateway(std::span<const uint8_t, 4> gateway)
{
    return Write(W5500_Reg::GAR0, W5500_Reg::Block::COMMON, gateway);
}


BareM_Status W5500::GetMacAddress(std::span<uint8_t, 6> mac)
{
    return Read(W5500_Reg::SHAR0, W5500_Reg::Block::COMMON, mac);
}

BareM_Status W5500::GetIPAddress(std::span<uint8_t, 4> ip)
{
    return Read(W5500_Reg::SIPR0, W5500_Reg::Block::COMMON, ip);
}

BareM_Status W5500::GetSubnetMask(std::span<uint8_t, 4> mask)
{
    return Read(W5500_Reg::SUBR0, W5500_Reg::Block::COMMON, mask);
}

BareM_Status W5500::GetGateway(std::span<uint8_t, 4> gateway)
{
    return Read(W5500_Reg::GAR0, W5500_Reg::Block::COMMON, gateway);
}

// ============================================================================
// SOCKET BUFFER SIZE
// ============================================================================

BareM_Status W5500::SetSocketBufferSize(uint8_t socket, BufferSize txSize, BufferSize rxSize)
{
    if (!ValidSocket(socket))
        return BareM_Status::ERROR;

    const auto oldTx = txBufferSize[socket];
    const auto oldRx = rxBufferSize[socket];

    txBufferSize[socket] = txSize;
    rxBufferSize[socket] = rxSize;

    // Make sure the global 16-KB limits are respected.
    if (!ValidBufferAllocation())
    {
        txBufferSize[socket] = oldTx;
        rxBufferSize[socket] = oldRx;

        return BareM_Status::ERROR;
    }

    return ConfigureSocketBuffer(socket);
}


// ============================================================================
// DIAGNOSTIC
// ============================================================================

BareM_Status W5500::ReadVersion(uint8_t& version)
{
    return Read8(W5500_Reg::VERSIONR, W5500_Reg::Block::COMMON, version);
}

BareM_Status W5500::ReadPhyConfig(uint8_t& phyConfig)
{
    return Read8(W5500_Reg::PHYCFGR, W5500_Reg::Block::COMMON, phyConfig);
}

BareM_Status W5500::SocketGetStatus(uint8_t socket, uint8_t& status)
{
    if (!ValidSocket(socket))
        return BareM_Status::ERROR;

    return SocketRead8(socket, W5500_Reg::Sn_SR, status); 
}

BareM_Status W5500::GetSocketInterrupt(uint8_t socket, uint8_t& interrupt)
{
    if (!ValidSocket(socket))
        return BareM_Status::ERROR;

    return SocketRead8(socket, W5500_Reg::Sn_IR, interrupt);
}

////////////////////

BareM_Status W5500::SocketGetTxFreeSize(
    uint8_t socket,
    uint16_t& size)
{
    if (!ValidSocket(socket))
        return BareM_Status::ERROR;

    return GetStableTxFreeSize(socket, size);
}

// ============================================================================
// SOCKET OPEN
// ============================================================================

BareM_Status W5500::SocketOpen(uint8_t socket, Protocol protocol, uint16_t port)
{
    if (!ValidSocket(socket))
        return BareM_Status::ERROR; // Requested socket index is within the valid range (0 - 7) ?

    // Configure the Socket Mode Register (Sn_MR) with the desired protocol type (e.g., TCP, UDP, MACRAW, etc.)
    auto status = SocketWrite8(socket, W5500_Reg::Sn_MR, static_cast<uint8_t>(protocol));

    if (status != BareM_Status::OK)
        return status;

    // Set the source port for this socket (Sn_PORT0/Sn_PORT1)    
    status = SocketWrite16(socket, W5500_Reg::Sn_PORT0, port);   

    if (status != BareM_Status::OK)
        return status;

    // Finally issue the OPEN command to the Socket Command Register (Sn_CR)
    return SocketCommand(socket, W5500_Reg::Sn_CR_Command::OPEN);
}

// ============================================================================
// SOCKET CLOSE
// ============================================================================

BareM_Status W5500::SocketClose(uint8_t socket)
{
    if (!ValidSocket(socket))
        return BareM_Status::ERROR;

    return SocketCommand(socket, W5500_Reg::Sn_CR_Command::CLOSE);
}

// ============================================================================
// SOCKET GET DESTINATION
// ============================================================================

BareM_Status W5500::SocketGetDestination(
    uint8_t socket,
    std::span<uint8_t, 4> ip,
    uint16_t& port)
{
    if (!ValidSocket(socket))
        return BareM_Status::ERROR;

    auto status = Read(
        W5500_Reg::Sn_DIPR0,
        W5500_Reg::SocketRegBlock(socket),
        ip);

    if (status != BareM_Status::OK)
        return status;

    return SocketRead16(
        socket,
        W5500_Reg::Sn_DPORT0,
        port);
}

// ============================================================================
// SOCKET SET DESTINATION
// ============================================================================

BareM_Status W5500::SocketSetDestination(uint8_t socket, std::span<const uint8_t, 4> ip, uint16_t port)
{
    if (!ValidSocket(socket))
        return BareM_Status::ERROR;

    // Write the 4-byte destination IP address (Sn_DIPR0 to Sn_DIPR3)
    auto status = Write(W5500_Reg::Sn_DIPR0, W5500_Reg::SocketRegBlock(socket), ip);

    if (status != BareM_Status::OK)
        return status;

    // Set the 16-bit destination port (Sn_DPORT0 / Sn_DPORT1)
    return SocketWrite16(socket, W5500_Reg::Sn_DPORT0, port);
}


// ============================================================================
// SOCKET SEND
// ============================================================================

BareM_Status W5500::SocketSend(uint8_t socket, std::span<const uint8_t> data)
{
    if (!ValidSocket(socket) || data.empty())
        return BareM_Status::ERROR;

    // ---------------------------------------------------------
    // Sn_TX_FSR must be read until stable.
    // ---------------------------------------------------------

    uint16_t freeSize;

    auto status = GetStableTxFreeSize(socket, freeSize);

    if (status != BareM_Status::OK)
        return status;

    if (data.size() > freeSize)
        return BareM_Status::ERROR;

    // ---------------------------------------------------------
    // Read current TX write pointer.
    // ---------------------------------------------------------

    uint16_t writePointer;

    status = SocketRead16(socket, W5500_Reg::Sn_TX_WR0, writePointer);

    if (status != BareM_Status::OK)
        return status;

    // ---------------------------------------------------------
    // IMPORTANT: Do NOT mask the pointer or split the transfer ourselves.
    // W5500 uses the raw 16-bit offset and handles the
    // socket-buffer addressing internally.
    // ---------------------------------------------------------

    status = Write(writePointer, W5500_Reg::SocketTxBlock(socket), data);

    if (status != BareM_Status::OK)
        return status;

    // ---------------------------------------------------------
    // Increment pointer. uint16_t naturally keeps the lower
    // 16 bits when the pointer crosses 0xFFFF.
    // ---------------------------------------------------------

    writePointer = static_cast<uint16_t>(writePointer + data.size());

    status = SocketWrite16(socket, W5500_Reg::Sn_TX_WR0, writePointer);

    if (status != BareM_Status::OK)
        return status;

    // ---------------------------------------------------------
    // Tell W5500 to transmit the data.
    // ---------------------------------------------------------

    return SocketCommand(socket, W5500_Reg::Sn_CR_Command::SEND);
}


// ============================================================================
// SOCKET RECEIVE
// ============================================================================

BareM_Status W5500::SocketReceive(uint8_t socket, std::span<uint8_t> data)
{
    if (!ValidSocket(socket) || data.empty())
        return BareM_Status::ERROR;

    // ---------------------------------------------------------
    // Sn_RX_RSR must be read until stable.
    // ---------------------------------------------------------

    uint16_t receivedSize;

    auto status = GetStableRxReceivedSize(socket, receivedSize);

    if (status != BareM_Status::OK)
        return status;

    if (data.size() > receivedSize)
        return BareM_Status::ERROR;

    // ---------------------------------------------------------
    // Read current RX read pointer.
    // ---------------------------------------------------------

    uint16_t readPointer;

    status = SocketRead16(socket, W5500_Reg::Sn_RX_RD0, readPointer);

    if (status != BareM_Status::OK)
        return status;

    // ---------------------------------------------------------
    // Use the raw W5500 pointer.
    // No software masking.
    // No software buffer splitting.
    // ---------------------------------------------------------

    status = Read(readPointer, W5500_Reg::SocketRxBlock(socket), data);

    if (status != BareM_Status::OK)
        return status;

    // ---------------------------------------------------------
    // Advance RX read pointer.
    // ---------------------------------------------------------

    readPointer = static_cast<uint16_t>(readPointer + data.size());

    status = SocketWrite16(socket, W5500_Reg::Sn_RX_RD0, readPointer);

    if (status != BareM_Status::OK)
        return status;

    // ---------------------------------------------------------
    // Notify W5500 that the data has been consumed.
    // ---------------------------------------------------------

    return SocketCommand(socket, W5500_Reg::Sn_CR_Command::RECV);
}
