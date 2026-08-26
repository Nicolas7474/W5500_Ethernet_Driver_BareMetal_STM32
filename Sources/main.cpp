/****************************************************************
 *************   Bare-Metal W5500 driver - Working example    ******
 ****************************************************************/

#include "spi.hpp"
#include "W5500.hpp"
#include "myConfig.h"
#include <stdint.h>
#include "W5500_registers.hpp"
#include <array>



int main(void)
{
	// ---------------------------------------------------------
	// MCU clock configuration
	// ---------------------------------------------------------
	SysClockConfig();
	SysTick_Init();
	GPIO_Config();
	RTC_Init();

	volatile int i = 0;
	while (i < 30000) { i = i + 1; } // Do nothing, just burn cycle

	GPIOE->BSRR = (1U << 0); // PE0 kaputt anyway
	GPIOE->BSRR = (1U << 1); // Turns off yellow leds
	GPIOE->BSRR = (1U << 2); // blue
	GPIOE->BSRR = (1U << 3); // turns off bright green

	// ---------------------------------------------------------
	// SPI3 initialization	//
	// Spi3_LowLevelInit() configures the SPI3 pins, W5500 CS
	// and W5500 RESET, and SpiDriver::Init() configures SPI3 and its DMA.
	// ---------------------------------------------------------

	if (spi3.Init(BaudRatePrescaler::DIV_2) != BareM_Status::OK) // DIV_4 -> 5.625  MHz
	{
		GPIOE->ODR ^= (1U << 1); // PE1 jaune
		for (;;);
	}

	i = 0;
	while (i < 30000) { i = i + 1; }
	// Do nothing, just burn cycles

	if (W5500::Init() != BareM_Status::OK)
	{
		GPIOE->ODR ^= (1U << 2); // PE2 bleu
		for (;;);
	}

	// ---------------------------------------------------------
	// W5500 is alive.
	// Nothing else for the first hardware test.
	// ---------------------------------------------------------
	for (;;)
	{
		 // Toggle PE1 LED
		    	    GPIOE->ODR ^= (1U << 1);

		    	    // Simple software delay loop
		    	    i = 0;
		    	    while (i < 3000000) { i = i + 1;}

	}
}


extern "C" void RTC_WKUP_IRQHandler(void)
{
    if (RTC->ISR & RTC_ISR_WUTF)
    {
        // Toggle PE3 LED
        GPIOE->ODR ^= (1U << 3);

        // Clear WUTF (write 0 to WUTF, 1 to write-1-to-clear or read-only bits)
        RTC->ISR = ~(RTC_ISR_WUTF) | (RTC->ISR & RTC_ISR_INIT);
    }

    // Clear EXTI Line 22 pending bit
    EXTI->PR = (1U << 22);
}





/*
F-RAM ATTEMPT TO READ / CHECK

if (spi4.Init(BaudRatePrescaler::DIV_16) != BareM_Status::OK) // DIV_4 -> 5.625  MHz
	{
		GPIOE->ODR ^= (1U << 2); // PE1 jaune
		for (;;);
	}


//	uint8_t tx_rdsr[2] = {0x05, 0x00};
//	uint8_t rx1[2]     = {0x00, 0x00};
//	uint8_t rx2[2]     = {0x00, 0x00};
//	uint8_t tx_wren    = 0x06;
//	uint8_t rx_dummy   = 0x00;
//
//	// Step 1: Initial Read Status Register
//	SpiDriver::CS_FRAM_Low();
//	spi4.TransmitReceive(std::span<const uint8_t>(tx_rdsr, 2), std::span<uint8_t>(rx1, 2), 10);
//	SpiDriver::CS_FRAM_High();

//	// Step 2: Set Write Enable Latch
//	SpiDriver::CS_FRAM_Low();
//	spi4.TransmitReceive(std::span<const uint8_t>(&tx_wren, 1), std::span<uint8_t>(&rx_dummy, 1), 10);
//	// Step 3: Read Status Register Again
//
//	spi4.TransmitReceive(std::span<const uint8_t>(tx_rdsr, 2), std::span<uint8_t>(rx2, 2), 10);
//	SpiDriver::CS_FRAM_High();

	// VERIFICATION:
	// rx1[1] MUST be 0x00
	// rx2[1] MUST be 0x02

//	if(Test_FRAM_FullDuplex() == false)
//		GPIOE->ODR ^= (1U << 1); // PE1 jaune


static bool Test_FRAM_Status()
{
    uint8_t txBuf[2] = {0x05, 0x00};   // RDSR + dummy
    uint8_t rxBuf[2] = {0x00, 0x00};

    SpiDriver::CS_FRAM_Low();

    BareM_Status status = spi4.TransmitReceive(
        std::span<const uint8_t>(txBuf, 2),
        std::span<uint8_t>(rxBuf, 2),
        10
    );

    SpiDriver::CS_FRAM_High();

    if (status != BareM_Status::OK)
        return false;

    return rxBuf[1] == 0x00;
}

static bool Test_FRAM_WREN_RDSR() {
    uint8_t tx_wren = 0x06;
    uint8_t rx_dummy = 0x00;

    // Step 1: Send Write Enable (0x06)
    SpiDriver::CS_FRAM_Low();
    spi4.TransmitReceive(
        std::span<const uint8_t>(&tx_wren, 1),
        std::span<uint8_t>(&rx_dummy, 1),
        10
    );
    SpiDriver::CS_FRAM_High(); // CS MUST toggle high to latch WREN

    // Small delay to ensure CS high-time requirement (tCSH min 10ns)
    for (volatile int i = 0; i < 100; i++);

    // Step 2: Read Status Register (0x05)
    uint8_t tx_rdsr[2] = {0x05, 0x00};
    uint8_t rx_rdsr[2] = {0x00, 0x00};

    SpiDriver::CS_FRAM_Low();
    spi4.TransmitReceive(
        std::span<const uint8_t>(tx_rdsr, 2),
        std::span<uint8_t>(rx_rdsr, 2),
        10
    );
    SpiDriver::CS_FRAM_High();

    // Verification:
    // rx_rdsr[1] should have Bit 1 set (0x02).
    // If rx_rdsr[1] is still 0xFF, MISO is not driving low.
    return (rx_rdsr[1] == 0x02);
}
*/
