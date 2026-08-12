//#include "stm32f446xx.h"
//#include "stm32f4xx.h"

#include <myConfig.h>


void activateFPU(void) {

#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));  /* set CP10 and CP11 Full Access */

	// Enable Lazy Stacking for better ISR performance : When an interrupt (like your TIM7 or TIM8 ISRs) occurs, the processor normally has to save all the FPU registers
	// to the stack. This is slow. Lazy Stacking tells the hardware only to save FPU registers if the ISR actually performs a floating-point operation.
	FPU->FPCCR |= FPU_FPCCR_LSPEN_Msk;
#endif
	// Enabling the hardware bits isn't enough; you must also tell your compiler (GCC, Clang, or Keil) to actually generate FPU instructions instead of using slow software libraries.
	// If you are using GCC (arm-none-eabi-gcc), add these flags to your build command:
	// -mfloat-abi=hard: Uses the hardware FPU for calculations and passing arguments.
	// -mfpu=fpv4-sp-d16: Specifies the specific FPU version on the STM32F4.
}

uint32_t SystemCoreClock = 180000000;
volatile uint32_t msTicks = 0; // Volatile ensures the compiler doesn't optimize out reads of this value

/*2. The Configuration
In your initialization code, you need to configure SysTick to trigger an interrupt every 1 millisecond.
Assuming your System Core Clock (HCLK) is already configured (e.g., to 180 MHz), you can use the CMSIS function SysTick_Config.
*/
void SysTick_Init() {
    // Configure SysTick to generate an interrupt every 1ms
    // The formula is: ticks = SystemCoreClock / 1000
    if (SysTick_Config(SystemCoreClock / 1000)) {
    	// The SysTick_Config function returns a value (typically 0 for success and 1 for failure
        // Capture error (should not happen with valid clock)
    	NVIC_SystemReset(); // Force the whole chip to reboot and try again
    }
}

/*3. The Handler (ISR)
The SysTick handler is predefined in the vector table. You just need to define it and increment your counter.
 */
void SysTick_Handler(void) {
	msTicks++;
}

/*4. The GetTick Equivalent : now, create a function to return that value. This is a direct replacement for HAL_GetTick().*/
uint32_t GetSysTick(void) {
	return msTicks;
}



void SysClockConfig(void)
{
#define PLL_M  25
#define PLL_N  360
#define PLL_P  0       // PLLP = 2

    // 1. Enable HSE and wait until ready
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    // 2. Enable power interface clock and voltage scaling
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS;

    // 3. Configure Flash
    FLASH->ACR = FLASH_ACR_ICEN |
                 FLASH_ACR_DCEN |
                 FLASH_ACR_PRFTEN |
                 FLASH_ACR_LATENCY_5WS;

    // 4. Configure bus prescalers

    // AHB = SYSCLK / 1 = 180 MHz
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

    // APB1 = HCLK / 4 = 45 MHz
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

    // APB2 = HCLK / 2 = 90 MHz
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;

    // 5. Configure main PLL
    RCC->PLLCFGR = (PLL_M << 0) |
                   (PLL_N << 6) |
                   (PLL_P << 16) |
                   RCC_PLLCFGR_PLLSRC_HSE;

    // 6. Enable PLL and wait until ready
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // 7. Select PLL as system clock
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}


void GPIO_Config(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    // PE3 = Heartbeat LED, active low
    GPIOE->MODER &= ~(3 << GPIO_MODER_MODER3_Pos);
    GPIOE->MODER |=  (1 << GPIO_MODER_MODER3_Pos);

    // LED OFF
    GPIOE->BSRR = GPIO_BSRR_BS3;
}


void RTC_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    PWR->CR |= PWR_CR_DBP; // Enable access to backup domain

    RCC->CSR |= RCC_CSR_LSION;
    while (!(RCC->CSR & RCC_CSR_LSIRDY));   // Enable LSI

    RCC->BDCR &= ~RCC_BDCR_RTCSEL;
    RCC->BDCR |= RCC_BDCR_RTCSEL_1;    // Select LSI as RTC clock

    RCC->BDCR |= RCC_BDCR_RTCEN;    // Enable RTC

    RTC->WPR = 0xCA;     // Disable RTC write protection
    RTC->WPR = 0x53;

    RTC->CR &= ~RTC_CR_WUTE;     // Disable wakeup timer

    while (!(RTC->ISR & RTC_ISR_WUTWF));     // Wait until configuration is allowed

    RTC->CR &= ~RTC_CR_WUCKSEL;
    RTC->CR |= RTC_CR_WUCKSEL_2;   // LSI ≈ 32 kHz / 16 = 2 kHz
    RTC->WUTR = 999;     // WUTR = 999 → approximately 2 Hz

    RTC->ISR &= ~RTC_ISR_WUTF;     // Clear wakeup flag

    RTC->CR |= RTC_CR_WUTIE;
    RTC->CR |= RTC_CR_WUTE;  // Enable wakeup interrupt + timer

    NVIC_SetPriority(RTC_WKUP_IRQn, 10);
    NVIC_EnableIRQ(RTC_WKUP_IRQn);

    RTC->WPR = 0xFF;    // Re-enable RTC write protection
}


