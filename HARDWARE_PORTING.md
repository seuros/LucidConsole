# Hardware Porting Guide

This guide walks you through adding support for a new ESP8266 board to LucidConsole. **Perfect for first-time contributors!**

## Overview

LucidConsole uses a board-agnostic architecture. Each board gets its own configuration in `boards/<board_name>/board.h` with pin definitions and feature flags.

## Prerequisites

**You MUST own the physical hardware.** This is not a research exercise - we need real testing with real boards to ensure quality.

## Step-by-Step Porting

### 1. Fork and Setup
```bash
git clone https://github.com/seuros/LucidConsole.git
cd LucidConsole
```

### 2. Create Board Definition
Copy an existing board as a template:
```bash
cp -r boards/ideaspark_oled_0.96_v2.1 boards/your_board_name
```

Edit `boards/your_board_name/board.h`:
```c
#ifndef BOARD_H
#define BOARD_H

// Board identification
#define BOARD_NAME "Your Board Name"
#define BOARD_VERSION "1.0"

// Essential pins (required)
#define UART_RX_PIN     3   // Usually GPIO3 (RX)
#define UART_TX_PIN     1   // Usually GPIO1 (TX)
#define LED_PIN         2   // Status LED

// Optional features
#define HAS_OLED        1   // Set to 0 if no display
#define OLED_SDA        4   // I2C data pin
#define OLED_SCL        5   // I2C clock pin

#define HAS_BUTTON      1   // Set to 0 if no button
#define BOOT_BUTTON_PIN 0   // Usually GPIO0

// Network configuration (you can change these)
#define AP_SSID         "YourBoard_UART"
#define AP_PASSWORD     "lucidconsole"

#endif
```

### 3. Pin Mapping Checklist

Essential pins to verify:
- [ ] **UART_RX_PIN** - Receives data from target device
- [ ] **UART_TX_PIN** - Sends data to target device (tri-stated by default!)
- [ ] **LED_PIN** - Status indication

Optional pins:
- [ ] **OLED_SDA/SCL** - Display pins (if your board has OLED)
- [ ] **BOOT_BUTTON_PIN** - Boot/flash button

### 4. Build and Test

Set your board name and build:
```bash
export IDF_PATH=/path/to/ESP8266_RTOS_SDK
export PATH="$PATH:/path/to/xtensa-lx106-elf/bin"
make BOARD=your_board_name
```

### 5. Critical Test: Stealth UART

**Most important test**: Verify your target device boots normally with LucidConsole connected:

1. Connect LucidConsole RX to target TX
2. Connect LucidConsole TX to target RX  
3. **Power up target device** - it should boot normally
4. LucidConsole should show WiFi network but **not interfere** with target boot
5. In web interface, click "Connect" to enable TX
6. Verify bi-directional communication

If target device fails to boot → TX pin is not properly tri-stated.

### 6. Features to Test

- [ ] ESP8266 boots and creates WiFi AP
- [ ] Web interface accessible at http://10.10.10.1
- [ ] LED blinks to show status
- [ ] UART RX receives data from target
- [ ] UART TX sends data when enabled (stealth mode working)
- [ ] OLED displays status (if present)
- [ ] Button functions work (if present)

### 7. Documentation

Add your board to `SUPPORTED_BOARDS.md`:
```markdown
| Your Board Name | @yourusername | #PR_NUMBER | Notes about quirks |
```

Include in your PR:
- **Photos of your board** connected to a target device
- **Serial console output** showing successful boot and WiFi connection
- **Web interface screenshot** at http://10.10.10.1 
- **UART test results** - proof that both RX and TX work
- **Pin mapping table** - document your pin choices
- **Build instructions** - exact commands that worked for you

**No hardware testing = No merge.** Pinout research alone is not sufficient.

## Common Issues

### Boot Loops
- Check flash mode in your build command (use `--flash_mode dio`)
- Verify power supply can handle WiFi radio power draw

### Target Device Won't Boot
- TX pin not tri-stated properly - check GPIO configuration
- Wrong pin mapping - verify with multimeter/scope

### WiFi Not Visible
- Check antenna connection
- Verify power supply stability
- Check for GPIO conflicts

### OLED Not Working
- I2C pins might conflict with other functions
- Try different SDA/SCL pins
- Set `HAS_OLED 0` to disable if problematic

## Getting Help

- Comment on your claimed issue - I'll help debug
- Include photos, error messages, and what you've tried
- Don't hesitate to ask "stupid" questions - we've all been there!

## Success Criteria

Your port is ready when:
1. Target device boots normally with LucidConsole attached
2. WiFi AP appears and web interface loads
3. UART bridge works bidirectionally
4. You've documented any board-specific quirks

Remember: **One board, one person, one great learning experience!**