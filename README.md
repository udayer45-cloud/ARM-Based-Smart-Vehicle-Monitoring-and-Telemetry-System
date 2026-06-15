# ARM-Based Smart Vehicle Monitoring and Telemetry System

An embedded system built on the **NXP LPC2129 (ARM7TDMI-S)** microcontroller that continuously monitors vehicle parameters — speed, temperature, ambient light — along with real-time clock data, and streams readings to both a 16×2 LCD and a PC serial terminal.

---

## Features

- **Real-time sensor acquisition** triggered by Timer1 interrupt every 1 second
- **Speed simulation** via onboard potentiometer → 0–120 kmph
- **Temperature sensing** with LM35 (onboard ADC, linear 10 mV/°C)
- **Light level sensing** with LDR through MCP3204 SPI external ADC (0–100 %)
- **Real-time clock** via DS1307 over I²C (time + date, BCD)
- **Dual LCD display modes** toggled by button press (EXT0 interrupt):
  - **Mode 0** — Sensor data: Speed, Voltage, Temp, LDR %
  - **Mode 1** — RTC data: Time (HH:MM:SS AM/PM) + Date (Day DD/MM/YYYY)
- **UART telemetry** to PC at 9600 baud (all values every second)

---

## Hardware

| Component | Interface | Notes |
|---|---|---|
| LPC2129 | — | 60 MHz (12 MHz XTAL + PLL), 3.3 V |
| DS1307 RTC | I²C (P0.2/P0.3) | BCD time/date, 5V supply via GPIO |
| MCP3204 | SPI0 (P0.4-P0.7) | 12-bit, 4-ch, CH2 = LDR |
| LM35 | On-chip ADC CH1 | 500 mV + 10 mV/°C |
| Potentiometer | On-chip ADC CH2 | Simulates vehicle speed |
| 16×2 LCD | GPIO Port1 (4-bit) | P1.17-P1.23 |
| UART0 | P0.0/P0.1 | 9600-8N1 to PC terminal |
| Push button | P0.16 (EXT0) | Toggles LCD view mode |

---

## Block Diagram

```
DC 3.3V Power Supply
        │
        ▼
┌───────────────────────┐
│   LPC2129 (60 MHz)    │◄──── Button (EXT0, P0.16)
│                       │
│  GPIO  ────────────►  16×2 LCD (4-bit, P1.17-P1.23)
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

## UART Output Format

Every second, the following is printed on the PC terminal:

```
Time and Date
HH:MM:SS AM DAY : WED DATE : 23/04/2026
Speed : 75 kmph Vout : 1.2 v
TEMP : 27 C LDR : 63%
```

---

## Software Architecture

```
main.c
├── Initialise all peripherals
├── Set DS1307 initial time/date
└── while(1)
    └── if f1==1 (Timer1 tick)
        ├── rtc_time()   → UART + LCD (if f2=1)
        ├── rtc_date()   → UART + LCD (if f2=1)
        ├── adc_read(2)  → Speed (potentiometer)
        ├── adc_read(1)  → Temperature (LM35)
        ├── mcp3204_adc_read(2) → LDR %
        ├── UART output (all values)
        └── LCD update (if f2=0, sensor view)

Interrupts
├── Timer1 ISR (1 s)  → f1 = 1
└── EXT0   ISR (btn)  → f2 ^= 1  (toggle LCD mode)
```

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

## Toolchain & Setup

| Tool | Version / Notes |
|---|---|
| IDE | Keil µVision 5 |
| Compiler | ARMCC V5 (not V6) |
| Target device | LPC2129 |
| Device pack | Keil.LPC2000_DFP |
| Flash tool | Flash Magic (UART ISP) |
| Simulation | Proteus 8 |

### Keil Project Setup

1. Create new project → target device: `LPC2129`
2. Add all `.c` files from `src/`
3. Add `include/` to include paths: `../include` or `.\include`
4. Set CCLK to `60000000` in target options
5. Select **ARMCC V5** (Project → Options → C/C++ → not "Use default compiler version 6")
6. Build and flash via Flash Magic at 9600 baud

---

## DS1307 Initial Time/Date

Set in `main.c` before the main loop. Modify these values before flashing:

```c
i2c_write(0xD0, 0x00, 0x00);   // Seconds  = 00
i2c_write(0xD0, 0x01, 0x45);   // Minutes  = 45  (BCD)
i2c_write(0xD0, 0x02, 0x09);   // Hours    = 09  (BCD, 24h)
i2c_write(0xD0, 0x03, 0x04);   // Day      = 4 (Wednesday)
i2c_write(0xD0, 0x04, 0x23);   // Date     = 23 (BCD)
i2c_write(0xD0, 0x05, 0x04);   // Month    = 04 (BCD)
i2c_write(0xD0, 0x06, 0x26);   // Year     = 26 → 2026 (BCD)
```

---

## Author

Naveen — B.Tech EEE, JNTUA College of Engineering Kalikiri (2025)  
Embedded Systems Training: Vector India, Bengaluru
