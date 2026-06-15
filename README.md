# ARM-Based Smart Vehicle Monitoring and Telemetry System

Bare-metal firmware for the **NXP LPC2129** (ARM7TDMI-S) that continuously monitors vehicle parameters — speed, temperature, ambient light — along with real-time clock data, and streams readings to both a 16×2 LCD and a PC serial terminal.

---

## Features

- Timer1 interrupt-driven sensor acquisition every 1 second
- Speed simulation via onboard potentiometer → 0–120 kmph
- Temperature sensing with LM35 (on-chip ADC, 10 mV/°C, 500 mV offset)
- Light level sensing with LDR through MCP3204 SPI external ADC (0–100 %)
- Real-time clock via DS1307 over I²C (time + date, BCD)
- Dual LCD display modes toggled by EXT0 button interrupt:
  - Mode 0 — Sensor view: Speed, Vout, Temp, LDR %
  - Mode 1 — RTC view: Time (HH:MM:SS AM/PM) + Date (Day DD/MM/YYYY)
- UART0 telemetry to PC at 9600-8N1 (all values every second)

---

## Hardware

| Component | Interface | Notes |
|-----------|-----------|-------|
| NXP LPC2129 | — | 60 MHz (12 MHz XTAL + PLL ×5), ARM7TDMI-S |
| LM35 | On-chip ADC CH1 | 500 mV + 10 mV/°C |
| Potentiometer | On-chip ADC CH2 | Simulates vehicle speed 0–120 kmph |
| MCP3204 | SPI0 (P0.4–P0.7) | 12-bit 4-ch ADC, CH2 = LDR |
| DS1307 RTC | I²C (P0.2/P0.3) | BCD time/date, 100 kHz |
| 16×2 LCD | GPIO Port1 (4-bit) | P1.17–P1.23 |
| UART0 | P0.0/P0.1 | 9600-8N1 to PC terminal |
| Push-button | P0.16 (EXT0) | Toggles LCD view mode |

---

## Block Diagram

```
DC 3.3V Power Supply
        │
        ▼
┌───────────────────────┐
│   LPC2129 (60 MHz)    │◄──── Button (EXT0, P0.16)
│                       │
│  GPIO  ────────────►  16×2 LCD (4-bit, P1.17–P1.23)
│                       │
│  UART0 ────────────►  PC Terminal (9600 bps)
│                       │
│  ADC CH1 ◄──────────  LM35 Temperature
│  ADC CH2 ◄──────────  Potentiometer (Speed)
│                       │
│  SPI0  ◄───────────►  MCP3204 ADC ◄── LDR
│                       │
│  I²C   ◄───────────►  DS1307 RTC
└───────────────────────┘
```

---

## Pin Map

### On-chip ADC
| Signal | Pin |
|--------|-----|
| LM35 (Temp) | AD0.1 (P0.28) |
| Potentiometer (Speed) | AD0.2 (P0.29) |

### SPI — MCP3204
| Signal | Pin |
|--------|-----|
| SCK | P0.4 |
| MISO | P0.5 |
| MOSI | P0.6 |
| CS (GPIO) | P0.7 |

### I²C — DS1307
| Signal | Pin |
|--------|-----|
| SCL | P0.2 |
| SDA | P0.3 |

### LCD (4-bit mode)
| LCD Pin | LPC2129 Pin |
|---------|-------------|
| RS | P1.17 |
| RW | P1.18 |
| EN | P1.19 |
| D4–D7 | P1.20–P1.23 |

### Interrupts
| Source | Pin | VIC Slot | Source Index |
|--------|-----|----------|--------------|
| Timer1 (1 s tick) | — | 5 | 5 |
| EXT0 (button) | P0.16 | 2 | 14 |

---

## Firmware Architecture

```
main()
 ├── Peripheral init: SPI → I²C → ADC → LCD → UART0
 ├── Interrupt init:  EXT0 → VIC/Timer1
 ├── DS1307 time set (hardcoded on every power-up)
 └── while(1)
      └── if f1 == 1  (set by Timer1 ISR every 1 s)
           ├── rtc_time() / rtc_date()   — I²C read + UART out
           ├── adc_read(2)               — speed (potentiometer)
           ├── adc_read(1)               — temperature (LM35)
           ├── mcp3204_adc_read(2)       — LDR light level
           ├── UART stream               — all values
           └── LCD update
                ├── f2=0 → sensor view (speed, Vout, temp, LDR)
                └── f2=1 → RTC view (handled inside rtc_time/rtc_date)

ISRs
 ├── Timer1_Handler()  → f1 = 1  (every 1 s)
 └── ext0_isr()        → f2 ^= 1 (toggle LCD mode)
```

---

## Sensor Signal Chains

**Temperature (LM35 → on-chip ADC CH1)**
```
Vout  = (res × 3.3) / 1023       [V]
Temp  = (Vout − 0.5) / 0.01      [°C]
```

**Speed (Potentiometer → on-chip ADC CH2)**
```
Speed = (res × 120) / 1023       [kmph, range 0–120]
```

**Light level (LDR → MCP3204 SPI ADC CH2)**
```
LDR % = (res × 100) / 4095       [%, range 0–100]
```

---

## UART Output Format

Printed every 1 second:
```
Time and Date
HH:MM:SS AM DAY : WED DATE : 23/04/2026
Speed : 75 kmph Vout : 1.2 v
TEMP : 27 C LDR : 63%
```

---

## Clock Configuration

| Parameter | Value |
|-----------|-------|
| XTAL | 12 MHz |
| PLL multiplier | ×5 |
| CCLK | 60 MHz |
| VPBDIV | 4 |
| PCLK | 15 MHz |

---

## Project Structure

```
ARM_Vehicle_Monitor/
├── include/
│   └── header.h              # Typedefs, struct rtc, all function prototypes
├── src/
│   ├── main.c                # Entry point, main loop, peripheral init
│   ├── delay.c               # delay_ms(), delay_sec() using Timer0
│   ├── adc_driver.c          # On-chip 10-bit ADC (LM35 + potentiometer)
│   ├── uart0_driver.c        # UART0 9600-8N1 TX/RX + integer/float helpers
│   ├── i2c_driver.c          # I²C master (100 kHz) for DS1307
│   ├── rtc_driver.c          # DS1307 read: rtc_time(), rtc_date(), bcd_to_dec()
│   ├── spi_driver.c          # SPI0 master (1 MHz) for MCP3204
│   ├── mcp3204_driver.c      # MCP3204 12-bit ADC read (single-ended)
│   ├── lcd_4bit_driver.c     # 16×2 LCD 4-bit driver (Port 1)
│   └── interrupts_driver.c   # Timer1 + EXT0 ISRs and VIC config
├── docs/
│   └── system_overview.md    # Detailed technical notes
└── README.md
```

---

## Source Files

| File | Responsibility |
|------|---------------|
| `main.c` | Entry point, main loop, sensor scaling |
| `header.h` | Typedefs, struct `rtc`, all function prototypes |
| `adc_driver.c` | On-chip 10-bit ADC (ADCR polling) |
| `spi_driver.c` | SPI0 master, ~1 MHz |
| `mcp3204_driver.c` | MCP3204 3-byte SPI protocol, 12-bit result |
| `i2c_driver.c` | I²C master (hardware peripheral, 100 kHz) |
| `rtc_driver.c` | DS1307 read, BCD→decimal, UART/LCD output |
| `lcd_4bit_driver.c` | 16×2 LCD, 4-bit mode, Port 1 |
| `uart0_driver.c` | UART0 TX/RX, integer and float helpers |
| `interrupts_driver.c` | VIC setup, Timer1 ISR, EXT0 ISR |
| `delay.c` | Timer0-based blocking delays (ms / sec) |

---

## Toolchain & Setup

| Tool | Details |
|------|---------|
| IDE | Keil µVision 5 |
| Compiler | **ARMCC V5** (not V6 — V6 breaks legacy LPC21xx headers) |
| Target device | LPC2129 |
| Device pack | Keil.LPC2000_DFP |
| Flash tool | Flash Magic (UART ISP) |
| Simulation | Proteus 8 |

### Keil Project Setup

1. Create new project → select target device: **LPC2129**
2. Add all `.c` files from `src/`
3. Add `include/` to include paths: `.\include`
4. Set CCLK to **60000000** in target options
5. Go to Project → Options → C/C++ → set compiler to **ARMCC V5** (not "Use default compiler version 6")
6. Build → flash via Flash Magic at 9600 baud

---

## DS1307 Initial Time/Date

Set in `main.c` before the main loop. Modify these before flashing:

```c
i2c_write(0xD0, 0x00, 0x00);   // Seconds  = 00
i2c_write(0xD0, 0x01, 0x45);   // Minutes  = 45  (BCD)
i2c_write(0xD0, 0x02, 0x09);   // Hours    = 09  (BCD, 24h)
i2c_write(0xD0, 0x03, 0x04);   // Day      = 4 (Wednesday, 1=Sun)
i2c_write(0xD0, 0x04, 0x23);   // Date     = 23 (BCD)
i2c_write(0xD0, 0x05, 0x04);   // Month    = 04 (BCD)
i2c_write(0xD0, 0x06, 0x26);   // Year     = 26 → 2026 (BCD)
```

> **Note:** Time resets on every power cycle. See Known Limitations #4 for the fix.

---

## Known Limitations

1. **`lcd_int()` negative bug** — calls `lcd_cmd(0x80)` (cursor reset) instead of printing `'-'`. No impact here since temperature is always positive, but fix before reuse.
2. **`uart0_float()` precision** — one decimal digit only. Fine for Vout display, not for precision measurements.
3. **`i2c_read()` address XOR** — XORs `sa` with 1 during the write phase (produces read address 0xD1 instead of write address 0xD0). Works on DS1307 because it ACKs both addresses, but **will break on other I²C devices**. Fix: send `sa` unmodified for the write phase, then `sa | 1` for the repeated-start read phase.
4. **DS1307 time hardcoded** — resets on every power cycle. Fix: check the CH bit (reg 0x00 bit 7) at startup; only write initial time if CH=1 (clock was halted/unset).

---

## Author

**Chalampalem Uday Kiran**
B.Tech EEE, JNTUA College of Engineering, Kalikiri (2025)
Advanced Embedded Systems Training — Vector India Pvt. Ltd., Bengaluru

GitHub: [github.com/udayer45-cloud](https://github.com/udayer45-cloud)
LinkedIn: [linkedin.com/in/udaykiran1807](https://linkedin.com/in/udaykiran1807)
