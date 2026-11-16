/*
 * Bare-metal blinky for
 * Green LED is on GPIO Port J, Pin 13
 */

#include <stm32f7xx.h> /** includes f769xx */

// Register addresses
#if 0
#define RCC_BASE      0x40023800
#define RCC_AHB1ENR   *(volatile uint32_t *)(RCC_BASE + 0x30)

#define GPIOJ_BASE    0x40022400
#define GPIOJ_MODER   *(volatile uint32_t *)(GPIOJ_BASE + 0x00)
#define GPIOJ_ODR     *(volatile uint32_t *)(GPIOJ_BASE + 0x14)
#endif

void DevEnv_SystemClock_Config(void);
void DevEnv_GPIO_Init(void);
void SysTick_Handler(void);

static volatile uint32_t ticks = 0;

// Helper for a simple delay
void DevEnv_delay(uint32_t ms) {
    uint32_t start = ticks;
    while ((ticks - start) < ms);
}

int main(void) {
    /** SystemInit() gets called in startup_stm32f769xx.s */

    DevEnv_SystemClock_Config();
    DevEnv_GPIO_Init();

    SysTick_Config(SystemCoreClock / 1000); /** at 216MHz now SysTick==1ms */

    // Loop forever
    while (1) {
        GPIOJ->ODR |= (1<<13);
        DevEnv_delay(500);
        GPIOJ->ODR &= ~(1<<13);
        DevEnv_delay(500);
    }

    return 1;
}

void DevEnv_SystemClock_Config(void) {
    // Enable HSE
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    // Enable power controller, set voltage scaling
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR1 |= PWR_CR1_VOS;

    // Overdrive for 216MHz
    PWR->CR1 |= PWR_CR1_ODEN;
    while (!(PWR->CSR1 & PWR_CSR1_ODRDY));
    PWR->CR1 |= PWR_CR1_ODSWEN;
    while (!(PWR->CSR1 & PWR_CSR1_ODSWRDY));

    // Configure PLL: HSE=25MHz, /M=25, *N=432, /P=2 → 216MHz
    RCC->PLLCFGR = (25  << RCC_PLLCFGR_PLLM_Pos) |
                   (432 << RCC_PLLCFGR_PLLN_Pos)  |
                   (0   << RCC_PLLCFGR_PLLP_Pos)  |
                   RCC_PLLCFGR_PLLSRC_HSE          |
                   (9   << RCC_PLLCFGR_PLLQ_Pos);

    // Enable PLL, wait for lock
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // Flash latency for 216MHz
    FLASH->ACR = FLASH_ACR_LATENCY_7WS |
                 FLASH_ACR_PRFTEN      |
                 FLASH_ACR_ARTEN;

    // AHB=216MHz, APB1=54MHz, APB2=108MHz
    RCC->CFGR = RCC_CFGR_HPRE_DIV1  |
                RCC_CFGR_PPRE1_DIV4  |
                RCC_CFGR_PPRE2_DIV2;

    // Switch to PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    SystemCoreClockUpdate();
}

void DevEnv_GPIO_Init(void) {
    /** Peripheral clock for GPIOJ */
    RCC->AHB1ENR |= (1 << 9);

    /** Configuring GPIOJ for general purpose output */
    GPIOJ->MODER &= ~(0b11 << 26);
    GPIOJ->MODER |= (0b01 << 26);
}

void SysTick_Handler(void) {
    ticks++;
}
