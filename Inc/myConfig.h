#include <stm32f446xx.h>

// If this header is being read by a C++ compiler, wrap the functions in extern "C"
#ifdef __cplusplus
extern "C" {
#endif

void SysTick_Handler(void);
void activateFPU(void);
void SysClockConfig (void);
void GPIO_Config(void);
void RTC_Init(void);
void SysTick_Init();
uint32_t GetSysTick(void);

#ifdef __cplusplus
}
#endif
