/**
 * @file    header.h
 * @brief   Global header — typedefs, struct definitions, and all driver
 *          function prototypes for the ARM-Based Smart Vehicle Monitoring
 *          and Telemetry System.
 *
 * Target MCU : NXP LPC2129 (ARM7TDMI-S), 60 MHz (PLL from 12 MHz XTAL)
 * Toolchain  : Keil µVision 5, ARMCC V5
 */

#ifndef HEADER_H
#define HEADER_H

#include <lpc21xx.h>

/* ------------------------------------------------------------------ */
/*  Convenience typedefs                                                */
/* ------------------------------------------------------------------ */
typedef unsigned char       uc;
typedef unsigned int        ui;
typedef unsigned short int  usi;

/* ------------------------------------------------------------------ */
/*  DS1307 RTC register map (packed in a struct)                        */
/* ------------------------------------------------------------------ */
struct rtc {
    uc s;     /**< Seconds   (BCD, reg 0x00) */
    uc m;     /**< Minutes   (BCD, reg 0x01) */
    uc h;     /**< Hours     (BCD, reg 0x02) — bit5 = AM/PM flag in 12h mode */
    uc day;   /**< Day-of-week (1=Sun … 7=Sat, reg 0x03) */
    uc date;  /**< Date      (BCD, reg 0x04) */
    uc mon;   /**< Month     (BCD, reg 0x05) */
    uc y;     /**< Year      (BCD, reg 0x06, last two digits) */
    uc me;    /**< AM/PM flag extracted from hours register bit 5 */
};

/* ------------------------------------------------------------------ */
/*  Delay utilities  (delay.c)                                          */
/* ------------------------------------------------------------------ */
void delay_sec(ui sec);
void delay_ms(ui ms);

/* ------------------------------------------------------------------ */
/*  On-chip ADC  (adc_driver.c)                                         */
/*  CH1 = LM35 temperature sensor                                       */
/*  CH2 = Onboard potentiometer (simulates vehicle speed)               */
/* ------------------------------------------------------------------ */
void         adc_init(void);
unsigned int adc_read(unsigned char ch);

/* ------------------------------------------------------------------ */
/*  UART0  (uart0_driver.c)  — 9600-8N1, to PC terminal                */
/* ------------------------------------------------------------------ */
void          uart0_init(ui baud);
void          uart0_tx(uc data);
uc            uart0_rx(void);
void          uart0_tx_string(char *p);
void          uart0_integer(int num);
void          uart0_float(float f);

/* ------------------------------------------------------------------ */
/*  I²C / DS1307 RTC  (i2c_driver.c, rtc_driver.c)                     */
/*  SCL = P0.2, SDA = P0.3                                              */
/*  DS1307 slave addr: 0xD0 (write) / 0xD1 (read)                     */
/* ------------------------------------------------------------------ */
void          i2c_init(void);
void          i2c_write(unsigned char sa, unsigned char mr, unsigned char data);
unsigned char i2c_read(unsigned char sa, unsigned char mr);

void          rtc_date(void);
void          rtc_time(void);
unsigned int  bcd_to_dec(int v);

/* ------------------------------------------------------------------ */
/*  SPI / MCP3204 External ADC  (spi_driver.c, mcp3204_driver.c)       */
/*  SCK = P0.4, MISO = P0.5, MOSI = P0.6, CS = P0.7                   */
/*  MCP3204: 12-bit, 4-channel, SPI mode 0,0                           */
/*  CH2 = LDR (light-dependent resistor)                               */
/* ------------------------------------------------------------------ */
void           spi_init(void);
unsigned char  spi_data(unsigned char data);
unsigned short int mcp3204_adc_read(unsigned char ch);

/* ------------------------------------------------------------------ */
/*  16×2 LCD — 4-bit mode  (lcd_4bit_driver.c)                         */
/*  Port 1 pins P1.17-P1.23 (data nibbles + EN/RW/RS)                  */
/* ------------------------------------------------------------------ */
void lcd_data(unsigned char data);
void lcd_cmd(unsigned char cmd);
void lcd_string(char *p);
void lcd_int(int n);
void lcd_float(float f);
void lcd_init(void);

/* ------------------------------------------------------------------ */
/*  Interrupt configuration  (interrupts_driver.c)                      */
/*  Timer1  — fires every 1 second  → sets f1 flag                    */
/*  EXT0    — button on P0.16       → toggles f2 LCD-mode flag         */
/* ------------------------------------------------------------------ */
void config_vic_for_timer1(void);
void config_timer1_intr(void);
void config_ext0(void);

#endif /* HEADER_H */
