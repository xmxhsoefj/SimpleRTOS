#include <stdint.h>

extern uint32_t _sidata;   /* Start of .data */
extern uint32_t _sdata;   /* Start of .data */
extern uint32_t _edata;   /* End of .data */
extern uint32_t _sbss;    /* Start of .bss */
extern uint32_t _ebss;    /* End of .bss */
extern uint32_t _estack;  /* Top of stack */

extern int main(void);
void Reset_Handler(void);
#if 0
void Default_Handler(void) {
    while(1);
}
#endif

uint32_t *vector_table[] __attribute__((section(".isr_vector"))) = {
    (uint32_t *) &_estack,
    (uint32_t *) Reset_Handler,
    // (uint32_t *) Default_Handler, // NMI
    // (uint32_t *) Default_Handler, // HardFault
    /** etc */
};

void Reset_Handler(void) {
    /** Copy .data from Flash to SRAM */
    uint32_t *pSrc = &_sidata;
    uint32_t *pDest = &_sdata;
    while (pDest < &_edata) {
        *pDest++ = *pSrc++;
    }

    /** Zero-fill .bss section */
    pDest = &_sbss;
    while (pDest < &_ebss) {
        *pDest++ = 0;
    }

    main();
    while(1);
}
