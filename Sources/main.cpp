/****************************************************************
 *************   Bare-Metal W5500 driver - Working example    ******
 ****************************************************************/

#include "W5500.hpp"
#include "W5500_registers.hpp"
#include "myConfig.h"
#include "spi.hpp"
#include <array>
#include <stdint.h>


uint8_t rxBuffer[256]{};
uint16_t receivedSize = 0;
volatile bool w5500Interrupt = false;


int main(void) {
  // MCU clock configuration
  SysClockConfig();
  SysTick_Init();
  GPIO_Config();
  RTC_Init();

  volatile int i = 0;
  while (i < 3000) { i = i + 1; } // Delay

  GPIOE->BSRR = (1U << 0); // PE0 kaputt anyway
  GPIOE->BSRR = (1U << 1); // Turns off yellow leds
  GPIOE->BSRR = (1U << 2); // blue
  GPIOE->BSRR = (1U << 3); // turns off bright green

  // SPI3 initialization  - DIV_4 -> 5.625  MHz
  if (spi3.Init(BaudRatePrescaler::DIV_4) != BareM_Status::OK) {
    GPIOE->ODR ^= (1U << 1); // PE1 jaune
    for (;;)
      ;
  }

  i = 0; while (i < 1000) { i = i + 1; } // Delay
  
  ASSERT_OK(W5500::Init() != BareM_Status::OK);
  // W5500 is alive.

  i = 0; while (i < 1000) { i = i + 1; } // Delay

  const uint8_t mac[] = {0x02, 0x12, 0x34, 0x56, 0x78, 0x9A};
  const uint8_t ip[] = {192, 168, 137, 50};
  const uint8_t subnet[] = {255, 255, 255, 0};
  const uint8_t gateway[] = {192, 168, 137, 1};

  ASSERT_OK(W5500::SetMacAddress(mac) != BareM_Status::OK);
  ASSERT_OK(W5500::SetIPAddress(ip) != BareM_Status::OK);
  ASSERT_OK(W5500::SetSubnetMask(subnet) != BareM_Status::OK);
  ASSERT_OK(W5500::SetGateway(gateway) != BareM_Status::OK);

  i = 0; while (i < 2000) { i = i + 1; } // Delay

  // safety check - verify the network configuration registers
  uint8_t macRead[6]{}; // buffer to hold the network values
  uint8_t ipRead[4]{};
  uint8_t subnetRead[4]{};
  uint8_t gatewayRead[4]{};

  W5500::GetMacAddress(macRead); // read and fill the buffer
  W5500::GetIPAddress(ipRead);
  W5500::GetSubnetMask(subnetRead);
  W5500::GetGateway(gatewayRead);

  bool networkConfigOk = true;

  for (size_t i = 0; i < 6; ++i)
    networkConfigOk &= (macRead[i] == mac[i]);

  for (size_t i = 0; i < 4; ++i) {
    networkConfigOk &= (ipRead[i] == ip[i]);
    networkConfigOk &= (subnetRead[i] == subnet[i]);
    networkConfigOk &= (gatewayRead[i] == gateway[i]);
  }

  if (!networkConfigOk) {
    GPIOE->ODR ^= (1U << 2);
    for (;;)
      ;
  }

  i = 0; while (i < 2000) { i = i + 1; } // Delay  

  // Open UDP socket 0
  ASSERT_OK(W5500::SocketOpen(0, W5500::Protocol::UDP, 5000));

  // Enable the RECV interrupt & enable Socket 0 in SIMR
  ASSERT_OK(W5500::SocketSetInterruptMask(0, W5500_Reg::Sn_IR_Bits::RECV)); 
  ASSERT_OK(W5500::SetSocketInterruptMask(0x01)); 

  i = 0; while (i < 2000) { i = i + 1; } // Delay

  // Read the Socket Status Register after SocketOpen(). Expected: Sn_SR = 0x22
  uint8_t socketStatus = 0;
  ASSERT_OK(W5500::SocketGetStatus(0, socketStatus) != BareM_Status::OK);

  i = 0; while (i < 2000) { i = i + 1; } // Delay

  // send an UDP packet: add the W5500 UDP destination registers first (Destination port Sn_DPORT0 / ip Sn_DIPR0)
  const uint8_t pcIp[] = { 192, 168, 137, 1 };
  ASSERT_OK(W5500::SocketSetDestination(0, std::span<const uint8_t, 4>(pcIp), 5000));

  i = 0; while (i < 2000) { i = i + 1; } // Delay

  // Send packet
  const uint8_t message[] = "Hi ST";
  ASSERT_OK(W5500::SocketSend(0, std::span<const uint8_t>(message, sizeof(message) - 1))); 

  i = 0; while (i < 2000) { i = i + 1; } // Delay


  // ----------- WHILE LOOP --------------
  for (;;) {   

    if (w5500Interrupt)
    {
      w5500Interrupt = false;
      uint8_t socketIR = 0; // socketIR will contain the actual value of Socket 0's Sn_IR register

      ASSERT_OK(W5500::GetSocketInterrupt(0, socketIR));
      
      if (socketIR & W5500_Reg::Sn_IR_Bits::RECV)
      {
        ASSERT_OK(W5500::SocketGetReceivedSize(0, receivedSize));        

        if (receivedSize > sizeof(rxBuffer))
        {
            GPIOE->ODR ^= (1U << 2);
            for (;;);
        }

        if (receivedSize > 0)        
          ASSERT_OK(W5500::SocketReceive(0, std::span<uint8_t>(rxBuffer, receivedSize)));         

        GPIOE->ODR ^= (1U << 1);
        ASSERT_OK(W5500::ClearSocketInterrupt(0, W5500_Reg::Sn_IR_Bits::RECV));
      }

    }
    
  }
}


extern "C" void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR & (1U << 15))
    {    
      EXTI->PR = (1U << 15);  // Clear EXTI15 pending flag  
      // Defer W5500 SPI communication to the main loop.     
      w5500Interrupt = true;
    }
}

extern "C" void RTC_WKUP_IRQHandler(void) {
  if (RTC->ISR & RTC_ISR_WUTF) {
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

if (spi4.Init(BaudRatePrescaler::DIV_16) != BareM_Status::OK) // DIV_4 -> 5.625
MHz
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
//	spi4.TransmitReceive(std::span<const uint8_t>(tx_rdsr, 2),
std::span<uint8_t>(rx1, 2), 10);
//	SpiDriver::CS_FRAM_High();

//	// Step 2: Set Write Enable Latch
//	SpiDriver::CS_FRAM_Low();
//	spi4.TransmitReceive(std::span<const uint8_t>(&tx_wren, 1),
std::span<uint8_t>(&rx_dummy, 1), 10);
//	// Step 3: Read Status Register Again
//
//	spi4.TransmitReceive(std::span<const uint8_t>(tx_rdsr, 2),
std::span<uint8_t>(rx2, 2), 10);
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
