/****************************************************************
*************   Bare-Metal W5500 driver - Working example    ******
****************************************************************/

#include "spi.hpp"
#include "W5500.hpp"
#include "myConfig.h"

#include <stdint.h>
#include "W5500_registers.hpp"

int main(void)
{
    // ---------------------------------------------------------
    // MCU clock configuration
    // ---------------------------------------------------------
    SysClockConfig();

    // ---------------------------------------------------------
    // General GPIO configuration
    // ---------------------------------------------------------
    GPIO_Config();

    // ---------------------------------------------------------
    // SPI3 initialization
    //
    // Spi3_LowLevelInit() configures the SPI3 pins, W5500 CS
    // and W5500 RESET, and SpiDriver::Init() configures SPI3
    // and its DMA.
    // ---------------------------------------------------------
    if (spi3.Init(BaudRatePrescaler::DIV_4) != BareM_Status::OK) // DIV_4 -> 5.625  MHz
    {
        for (;;);
    }

    // ---------------------------------------------------------
    // W5500 hardware bring-up
    //
    // - Hold RESET for 50 ms
    // - Release RESET
    // - Wait 50 ms for W5500 startup
    // - Read VERSIONR
    // - Expect 0x04
    // ---------------------------------------------------------
    if (W5500::Init() != BareM_Status::OK)
    {
        for (;;);
    }

    // ---------------------------------------------------------
    // W5500 is alive.
    //
    // Nothing else for the first hardware test.
    // ---------------------------------------------------------
    for (;;)
    {
    }
}
