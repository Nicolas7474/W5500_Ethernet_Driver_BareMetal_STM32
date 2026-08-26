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
// Changed from 25 to 24 to maintain 1 MHz VCO input frequency
#define PLL_M  24
#define PLL_N  360
#define PLL_P  0       // PLLP bit field 00 = divider of 2

    // 1. Enable HSE and wait until ready
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    // 2. Enable power interface clock and voltage scaling
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS;

    // 3. Configure Flash (5 Wait States needed for 180 MHz at 3.3V)
    FLASH->ACR = FLASH_ACR_ICEN |
                 FLASH_ACR_DCEN |
                 FLASH_ACR_PRFTEN |
                 FLASH_ACR_LATENCY_5WS;

    // 4. Configure bus prescalers
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;   // AHB  = 180 MHz
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;  // APB1 = 45 MHz
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;  // APB2 = 90 MHz

    // 5. Configure main PLL
    RCC->PLLCFGR = (PLL_M << RCC_PLLCFGR_PLLM_Pos) |
                   (PLL_N << RCC_PLLCFGR_PLLN_Pos) |
                   (PLL_P << RCC_PLLCFGR_PLLP_Pos) |
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
	/*LED PE0*/
	// 2. Configure PE1 as General Purpose Output
	// Clear mode bits for Pin 1 (bits 3:2)
	GPIOE->MODER &= ~(3U << (0 * 2));
	// Set mode to Output (01 in binary) -> 1 << 2 = 4
	GPIOE->MODER |= (1U << (0 * 2));
	// Force PE0 to Push-Pull Output Mode
	GPIOE->OTYPER &= ~(1U << 0);

	/*LED PE1*/
	// 1. Enable GPIOE clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	// 2. Configure PE1 as General Purpose Output
	// Clear mode bits for Pin 1 (bits 3:2)
	GPIOE->MODER &= ~(3U << (1 * 2));
	// Set mode to Output (01 in binary) -> 1 << 2 = 4
	GPIOE->MODER |= (1U << (1 * 2));
	// 3. Optional: Set output type to Push-Pull (default is 0, which is push-pull)
	GPIOE->OTYPER &= ~(1U << 1);
	// 4. Optional: Set speed to low/medium
	GPIOE->OSPEEDR &= ~(3U << (1 * 2)); // Low speed is fine for blinking

	/*LED PE2*/
	// 2. Configure PE2 as General Purpose Output
	// Clear mode bits for Pin 1 (bits 3:2)
	GPIOE->MODER &= ~(3U << (2 * 2));
	// Set mode to Output (01 in binary) -> 1 << 2 = 4
	GPIOE->MODER |= (1U << (2 * 2));

	/*LED PE3*/
	// 2. Configure PE3 as General Purpose Output
	// Clear mode bits for Pin 1 (bits 3:2)
	GPIOE->MODER &= ~(3U << (3 * 2));
	// Set mode to Output (01 in binary) -> 1 << 2 = 4
	GPIOE->MODER |= (1U << (3 * 2));

	// PA10 config: Enable GPIOA clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	// PA10 = General purpose output
	GPIOA->MODER &= ~(3U << (10U * 2U));
	GPIOA->MODER |=  (1U << (10U * 2U));
	// Push-pull
	GPIOA->OTYPER &= ~(1U << 10);
	// High speed
	GPIOA->OSPEEDR &= ~(3U << (10U * 2U));
	GPIOA->OSPEEDR |=  (2U << (10U * 2U));
	// No pull-up / pull-down
	GPIOA->PUPDR &= ~(3U << (10U * 2U));
	// Initially low
	GPIOA->BSRR = (1U << (10U + 16U));
}


void RTC_Init(void)
{
    // 1. Enable Power and Backup Domain Access
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_DBP;

    // Reset Backup Domain
    RCC->BDCR |= RCC_BDCR_BDRST;
    RCC->BDCR &= ~RCC_BDCR_BDRST;

    // 2. Enable LSI and wait for ready
    RCC->CSR |= RCC_CSR_LSION;
    while (!(RCC->CSR & RCC_CSR_LSIRDY));

    // 3. Select LSI as RTC clock and enable RTC
    RCC->BDCR &= ~RCC_BDCR_RTCSEL;
    RCC->BDCR |= RCC_BDCR_RTCSEL_1;    // LSI selected (10)
    RCC->BDCR |= RCC_BDCR_RTCEN;

    // 4. Disable RTC write protection
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    // 5. Configure Wakeup Timer
    RTC->CR &= ~RTC_CR_WUTE;            // Disable wakeup timer
    while (!(RTC->ISR & RTC_ISR_WUTWF));// Wait until allowed

    // Clock selection: LSI (~32 kHz) / 16 = ~2000 Hz
    RTC->CR &= ~RTC_CR_WUCKSEL;
    RTC->WUTR = 999;                    // 2000 Hz / 1000 = 2 Hz (0.5s toggle)
    RTC->ISR &= ~RTC_ISR_WUTF;          // Clear wakeup flag

    // 6. Enable Wakeup Interrupt and Timer
    RTC->CR |= RTC_CR_WUTIE;
    RTC->CR |= RTC_CR_WUTE;

    RTC->WPR = 0xFF;                    // Re-enable RTC write protection

    // 7. Configure EXTI Line 22 for RTC Wakeup
    EXTI->IMR  |= (1U << 22);           // Unmask EXTI line 22
    EXTI->RTSR |= (1U << 22);           // Enable Rising Edge Trigger

    // 8. Enable Interrupt in NVIC
    NVIC_SetPriority(RTC_WKUP_IRQn, 10);
    NVIC_EnableIRQ(RTC_WKUP_IRQn);
}
