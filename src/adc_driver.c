/**
 * @file    adc_driver.c
 * @brief   On-chip 10-bit ADC driver for LPC2129.
 *
 * Channels used
 * ──────────────
 *  CH1 (AD0.1) → LM35 temperature sensor
 *      • Vout = 500 mV + 10 mV/°C
 *      • At 3.3 V reference: temp = (Vadc - 0.5) / 0.01  °C
 *
 *  CH2 (AD0.2) → Onboard potentiometer (speed simulation 0-120 kmph)
 *
 * PINSEL1 bits
 * ─────────────
 *  Bits [29:28] → AD0.3  (not used but set in original code — kept as-is)
 *  Bits [23:22] → AD0.2  = 01
 *  Bits [21:20] → AD0.1  = 01
 *
 * ADCR register
 * ──────────────
 *  [7:0]  SEL   — channel select (set dynamically in adc_read)
 *  [15:8] CLKDIV— ADC clock = PCLK/(CLKDIV+1), max 4.5 MHz
 *          0x04 here → PCLK/(4+1) = 15/5 = 3 MHz  ✓
 *  [21]   PDN   — 1 = ADC powered up
 *  [24]   START — 1 = start conversion (cleared after done)
 */

#include <LPC21xx.H>
#include "header.h"

/** DONE flag: bit 31 of ADDR is 1 when conversion is complete */
#define DON  ((ADDR >> 31) & 1)

/**
 * @brief  Initialise the on-chip ADC.
 *         Configures pin functions and powers up the ADC block.
 */
void adc_init(void)
{
    PINSEL1 |= 0x15400000;  /* AD0.1, AD0.2, AD0.3 alternate functions */
    ADCR     = 0x00200400;  /* PDN=1 (powered), CLKDIV=4 → ADC clk 3 MHz */
}

/**
 * @brief  Perform a single software-triggered ADC conversion.
 * @param  ch   ADC channel number (0–7).
 * @return 10-bit result (0–1023).
 */
unsigned int adc_read(unsigned char ch)
{
    int res;

    ADCR |=  (1 << ch);     /* Select channel */
    ADCR |=  (1 << 24);     /* Start conversion (START=1) */
    while (DON == 0);        /* Poll DONE bit */

    res   = (ADDR >> 6) & 0x3FF;   /* Bits [15:6] hold the 10-bit result */

    ADCR &= ~(1 << 24);     /* Clear START */
    ADCR &= ~(1 << ch);     /* Deselect channel */

    return res;
}
