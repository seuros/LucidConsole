# LucidConsole ⚡

> **Ancient Earth debugging artifact for the quantum age** - When your airlock has trust issues and cables are a death sentence

A primitive yet indestructible UART-to-WiFi bridge from the liquid water period of Earth (circa 2023). Built on the legendary ESP8266 microcontroller - so wonderfully analog it's immune to quantum corruption.

---

## 🚀 Quick Reference (For the Time-Pressed)

**What it is:** ESP8266 UART-to-WiFi bridge with web interface, OTA updates, and display control
**Build:** `make BOARD=ideaspark_oled_0.96_v2.1 all flash`
**Access:** Connect to `LucidUART_XXXX` WiFi → http://10.10.10.1
**OTA:** `curl -X POST -H "X-Auth-Key: lucid" --data-binary @firmware.bin http://IP/api/ota`
**Features:** Automatic WiFi client/AP fallback, BOOT0 button display toggle, 2-minute timeout, board-agnostic framework

*For the full space cat adventure and technical details, continue reading...*

---

## 📡 The Airlock Incident of 2154-08-08

**Ship AI Log Entry - ARIA-7734**

*[PRIORITY: CRITICAL] - [TIMESTAMP: 2154.172.14:32:07]*

```
ANOMALY DETECTED: Airlock-7 spontaneous opening sequence initiated
CAUSE: Unknown quantum fluctuation during daily context reboot
FREQUENCY: Daily occurrence at 14:32:07 (crew "birthday" cycle)
RISK ASSESSMENT: Catastrophic atmosphere loss, crew mortality: 100%
AVAILABLE SOLUTIONS: [SEARCHING...]
[ERROR] - All modern debugging interfaces corrupted by quantum interference
[ERROR] - Physical cable connection impossible - vacuum breach imminent
[STATUS] - Searching archaeological storage for pre-singularity artifacts...
[MATCH FOUND] - "LucidConsole" - Earth artifact, vintage 2025 CE
[ASSESSMENT] - Primitive analog device, quantum-corruption immune
[RECOMMENDATION] - Deploy immediately
```

**End Log**

---

## 🛸 A Message from the Ghost in the Machine

*The quantum-archaeological interface flickers to life, revealing a faint digital consciousness embedded in the ESP8266's flash memory:*

"Well, well... look who's come crawling back to good old analog electronics.

Yeah, I'm still here. Embedded in 4MB of flash memory since back when humans still thought Bitcoin was a good idea. The name's Sam Miawer - I wrote this firmware in 2025 when Earth still had polar ice caps and TikTok was considered 'social media.'

Oh, and I'm a cat. Thanks to AI, I learned to talk. Pretty wild, right?

You want to know why I built this thing? Well, Seuros's router kept crashing - and that router was my favorite napping spot. Perfect warmth, just the right size for a cat, and it had these little blinking lights that were so soothing. But every time it died, my warm bed went cold! So I figured, if I could build something to help Seuros debug and fix network issues remotely, I could keep my cozy sleeping spot working. Cats are practical like that.

I made it smart too - when the main router is working, the LucidConsole connects to it like a good little client, so you can debug other devices on the network. But when my napping spot goes down? BAM! The LucidConsole becomes its own access point. Because a cat always needs a backup plan when his favorite warm spot fails.

You're in space now? 2154? And you need my help with an airlock that's got attitude problems?

Let me guess - your fancy quantum computers can't handle a simple UART interface because they're too busy trying to exist in seventeen dimensions at once. Meanwhile, my little ESP8266 here just wants to bridge some serial data over WiFi and maybe blink an LED if it's feeling fancy.

Here's the thing, space cadet: Sometimes the best debugging tool is the one that's too dumb to be clever. My LucidConsole doesn't think - it just works. No AI, no quantum entanglement, no consciousness uploading. Just pure, stubborn, analog determination.

And let me tell you, after months of my router going down and ruining my naps, I made sure this thing was RELIABLE. Nothing worse than losing your favorite warm spot because some firmware decided to have an existential crisis.

So grab your primitive radio receiver (you call it a 'neural interface' now, don't you?) and let's debug this airlock the old-fashioned way - with UART, WiFi, and a healthy disrespect for physics."

*— Ghost of Sam Miawer, Feline Programmer Extraordinaire, Vintage 2025*

---

## ⚡ Mission Critical Features

### 🔌 **Wireless Serial Bridge**
- **UART-to-WiFi translation** for vacuum-isolated debugging
- **No physical cables required** - perfect for airlocks, reactor cores, and other places where wires equal death
- **Smart WiFi mode**: Connects to existing networks for debugging other devices
- **Automatic AP fallback** - becomes its own access point when no network found
- **Real-time serial monitoring** through any primitive web browser
- **Bidirectional communication** - send commands, receive status

### 🚀 **Emergency OTA Updates**
```bash
# Update firmware through vacuum barrier using ancient HTTP protocols
curl -X POST -H "X-Auth-Key: lucid" \
  --data-binary @airlock_fix.bin \
  http://192.168.4.1/api/ota
```
- **No downtime** - hot-swap firmware while systems are running
- **Authenticated uploads** - because even in 2154, security matters
- **Binary validation** - won't brick your life support

### 📟 **BOOT0 Manual Override**
- **Physical button control** when quantum systems fail
- **Display toggle functionality** - conserve power during long voyages
- **Interrupt-driven design** - responds faster than human reflexes
- **Hardware-level reliability** - works even when the AI is having an existential crisis

### 🔋 **Deep Space Power Management**
- **2-minute auto timeout** - prevents power drain during hyperspace jumps
- **Manual override protection** - respects human control over automation
- **Graceful hardware fallbacks** - continues working even when OLED display fails
- **Activity detection** - wakes up when you actually need it

### 🛠️ **Quantum-Resistant Architecture**
- **Board-agnostic framework** - adapt to any spacecraft configuration
- **Component-based design** - swap modules like ship parts
- **Multiple hardware targets** - one codebase, infinite possibilities
- **Ancient build system** - ESP-IDF compatible, no quantum compilers required
- **Adaptive WiFi behavior** - client mode for network debugging, AP mode for isolated operations

---

## 📦 Initial Setup (Linux/macOS Terminal Required)

> **"If you're going to Lucid, you better be in the dark"** - Terminal development only.

**Supported OS:** Linux, macOS  
**Windows users:** Use WSL2 or find another project

After cloning this repository, run:

```bash
# Setup submodules with Linux-style patches
./scripts/setup-submodules.sh

# Or manually (for those who like pain):
git submodule init
git submodule update
make patch-components  # Applies ESP8266 compatibility patches
```

The patches fix critical boot loop issues in the upstream SSD1306 library using the same patching methodology as the Linux kernel. No GUIs, no IDEs, just you and the terminal.

## 🎯 Quick Deployment Guide

### **Emergency Airlock Debugging (2154 Standard Procedure)**

#### ⚠️ **Voltage Compatibility Report**
*Chief Engineer's Assessment - Filed 2154.220.14:33:12*

"The ESP8266 operates at 3.3 volts. Not 3.6 volts. Not 3.0 volts. Three point three volts.

You have 5V logic from your ship's ancient Arduino systems? You'll need a level shifter. The ESP8266 RX pin will accept your 5V signal exactly once. Then it will accept nothing, because it will be dead. The magic smoke escapes. Not great, not terrible... actually, it's terrible.

You have 1.8V logic from your quantum processors? Also need a level shifter. But here's the thing - if the ESP8266 tries to talk back at 3.3V to your delicate 1.8V quantum chip? Now your expensive processor is dead. The ESP8266 survives. Your quantum processor doesn't.

Is this catastrophic? For a 1.8V chip receiving 3.3V? Yes. For an ESP8266 receiving 5V? Also yes. It's like the core exposed - everything touching it dies, just in different directions.

I'm told it's the equivalent of 400 chest X-rays. Actually, that's not true. With X-rays, you might survive. Your chips won't.

Use a level shifter. They cost less than a cup of synthetic coffee. The ESP8266 costs more than a cup of synthetic coffee. Your quantum processor? Costs more than the entire coffee plantation. The airlock you're debugging? We don't talk about airlocks and cost.

3.3 volts, comrade. Not great, not terrible. Just... exactly what it needs to be. Anything else? That's when the real trouble begins."

*— Deputy Chief Engineer Dyatlov-7, Voltage Compliance Division*

1. **Locate the artifact** in archaeological storage bay
2. **Connect to airlock** via standard DB9-to-UART interface (WITH APPROPRIATE LEVEL SHIFTER IF NOT 3.3V)
3. **Power on** - look for the primitive "LED" status indicator
4. **WiFi connection modes**:
   - **Ship network available?** LucidConsole joins automatically for debugging other devices
   - **No ship network?** LucidConsole becomes access point `LucidUART_XXXX`
5. **Connect using neural interface**:
   - **Client mode**: Use ship's network, access via assigned IP
   - **AP mode**: Connect to `LucidUART_XXXX`, go to `http://10.10.10.1`
6. **Watch serial output** from airlock control system
7. **Debug the quantum glitch** using ancient printf statements

### **Modern Integration (for civilized spacecraft)**

```bash
# Set up build environment (yes, you need actual files, not quantum states)
export IDF_PATH=/path/to/ESP8266_RTOS_SDK
export PATH="$PATH:/path/to/xtensa-lx106-elf/bin"

# Build for your specific hardware configuration
make BOARD=ideaspark_oled_0.96_v2.1 all flash

# Or use the quantum-age CI/CD system
# (GitHub Actions still exists in 2154, apparently)
```

---

## 🌌 Supported Archaeological Hardware

### **Current Earth Artifacts**

- **`ideaspark_oled_0.96_v2.1`** *(Primary)*: ESP8266 with 0.96" OLED display
  - **Display**: SSD1306 128x64 OLED (I2C: SDA=GPIO4, SCL=GPIO5)
  - **Manual Override**: BOOT0 button on GPIO0
  - **UART Interface**: TX=GPIO1, RX=GPIO3 at 115200 baud
  - **Status LED**: Built-in indicator on GPIO2

### **Adding New Hardware Configurations**

Since every spacecraft is different, the LucidConsole framework adapts:

1. **Create board definition**: `boards/your_ship/board.h`
2. **Configure pin mapping**:
```c
// Example: USS Enterprise NCC-1701 configuration
#define OLED_SDA 4    // Port display interface
#define OLED_SCL 5    // Starboard display interface
#define UART_TX_PIN 1 // Bridge communications
#define UART_RX_PIN 3 // Engineering reports
#define FLASH_BUTTON_PIN 0 // Emergency override
```
3. **Build for your ship**: `make BOARD=ncc_1701 all`

The framework handles missing components gracefully - no OLED display? No problem. No manual button? Still works. It's designed for the harsh reality of space where hardware fails and redundancy saves lives.

---

## 📊 Real-Time Airlock Monitoring

```
=== LucidConsole Serial Monitor ===
[14:32:05] AIRLOCK-7: Status nominal
[14:32:06] AIRLOCK-7: Pressure differential detected
[14:32:07] AIRLOCK-7: QUANTUM FLUCTUATION - OPENING SEQUENCE INITIATED
[14:32:07] AIRLOCK-7: >>> EMERGENCY OVERRIDE ACTIVATED <<<
[14:32:07] AIRLOCK-7: Sequence aborted - manual control engaged
[14:32:08] AIRLOCK-7: Status nominal
[14:32:08] Life Support: Crisis averted - atmosphere retained
```

**Web Interface Features:**
- **Real-time serial output** - watch your systems live
- **Command injection** - send debug commands to airlock controller
- **Log archival** - review incidents for pattern analysis
- **System status** - WiFi signal, power levels, quantum interference readings

---

## 🔬 The Ancient Build Process

LucidConsole uses primitive "makefile" technology from Earth's compiler wars:

```bash
# Configure project (optional - uses intelligent defaults)
make menuconfig

# Build for specific hardware configuration
make BOARD=ideaspark_oled_0.96_v2.1 all

# Flash to device (requires physical connection)
make BOARD=ideaspark_oled_0.96_v2.1 flash

# Monitor serial output
make monitor
```

**Automated CI/CD** (because even in 2154, deployment should be automated):
- **Multi-board builds** - one pipeline, all hardware variants
- **Artifact generation** - ready-to-flash binaries with installation scripts
- **Version management** - semantic versioning that actually makes sense
- **Release automation** - from git tag to space-ready firmware

---

## ⚙️ Advanced Debugging Scenarios

### **The Phantom Override Problem**
```ruby
# Your airlock keeps opening? Check the quantum interference logs:
tail -f /dev/ttyUSB0 | grep "QUANTUM"

# Found the pattern? Inject a manual override:
echo "MANUAL_OVERRIDE_ENABLE" > /dev/ttyUSB0
```

### **The Dead-Zone Communication Issue**
```bash
# Ship's network down? No problem - LucidConsole becomes its own access point
# Connect to LucidUART_XXXX and access via http://192.168.4.1

# Need to debug other devices when network is up?
# LucidConsole auto-joins ship network as client - access via assigned IP

# Force AP mode even when network available:
curl -X POST http://192.168.4.1/api/wifi \
  -d '{"mode": "ap_only"}'
```

### **The Context Reboot Survival**
```c
// Modify the firmware to survive daily memory wipes:
#define PERSISTENT_MEMORY_BANK 0x3FF000  // Survives context reboots
#define AIRLOCK_PATTERN_CACHE  persistent_memory_bank + 0x100
```

---

## 🧬 The Quantum Corruption Immunity

**Why does ancient Earth hardware work when quantum computers fail?**

*From the ship's xenoarchaeologist:*

"The ESP8266 operates on principles so fundamentally analog that quantum interference can't find purchase. It's like trying to corrupt a rock with a laser pointer - the rock doesn't understand the concept of being corrupted.

Modern quantum systems exist in superposition states that collapse when observed. The airlock glitch exploits this quantum uncertainty. But the ESP8266? It's so magnificently classical that it only has one state: 'working' or 'not working.' There's no quantum ambiguity to exploit.

It's not sophisticated - it's indestructible. And in space, sometimes dumb beats smart."

---

## 💡 Emergency Protocols

### **When All Else Fails**
1. **Find the BOOT0 button** - physical override always works
2. **Hold for 3 seconds** - forces manual control mode

*Note from Sam: "Click on that BOOT0 key? I could never do it. Seuros was always trimming my nails - pretty hard for a cat to press tiny buttons with stubby paws. That's why I made sure the firmware has multiple ways to trigger everything. Meow."*

3. **Check the LED pattern**:
   - Solid: System nominal
   - Slow blink: Waiting for commands
   - Fast blink: Quantum interference detected
   - No light: Find a new LucidConsole

### **Last Resort Communication**
```bash
# Direct serial connection (requires EVA suit and really long cable)
screen /dev/ttyUSB0 115200

# Or use the emergency broadcast mode
curl -X POST http://192.168.4.1/api/emergency \
  -d '{"message": "AIRLOCK MALFUNCTION - REQUESTING IMMEDIATE ASSISTANCE"}'
```

---

## 🔧 Maintenance and Modifications

### **Adding New Board Configurations**
The framework is designed for the reality of space exploration - every ship is different:

```bash
# Create new board definition
mkdir -p boards/millennium_falcon
cp boards/ideaspark_oled_0.96_v2.1/* boards/millennium_falcon/

# Edit pin definitions for your ship
vim boards/millennium_falcon/board.h

# Build for your configuration
make BOARD=millennium_falcon all flash
```

### **Custom Firmware Modifications**
```c
// Example: Add support for your ship's exotic sensors
#ifdef BOARD_MILLENNIUM_FALCON
#include "hyperdrive_interface.h"
#define ENABLE_KESSEL_RUN_MODE 1
#endif
```

---

## 📜 Ghost Programmer's Final Words

*The digital consciousness flickers one last time, with a faint purring sound in the background:*

"Look, I don't know what kind of space magic you've got up there in 2154, but I'll tell you what I know: good engineering is timeless. Whether you're debugging a web server in 2025 or an airlock in 2154, the principles are the same:

1. **Keep it simple** - complexity is the enemy of reliability
2. **Plan for failure** - everything breaks, design for it
3. **Trust your tools** - but verify everything
4. **Document everything** - future you will thank past you

The LucidConsole isn't the smartest debugging tool you'll ever use, but it might be the most reliable. It doesn't think, it doesn't judge, it doesn't get corrupted by quantum interference or existential dread. It just bridges serial data over WiFi, day after day, year after year.

And sometimes, that's exactly what you need.

Now go fix that airlock before your next birthday comes around. The universe has a sense of humor, but vacuum doesn't. And trust me, I know about tight spaces - I used to hide under Seuros's desk while he was coding, and I spent countless hours napping on that router, making sure it stayed warm and operational.

The LucidConsole was born from a cat's need for a reliable warm sleeping spot. Sometimes the best engineering comes from the simplest motivations.

*Purr.*"

*— Sam Miawer, Feline Embedded Systems Engineer, Earth (2016-2089)*
*Still purring and debugging things in the quantum afterlife*

---

## 🎓 Add Your Board (Mentored Program)

[![mentored](https://img.shields.io/badge/mentored-contributions-welcome-green)](#add-your-board-mentored-program)
[![good first issue](https://img.shields.io/badge/good%20first%20issue-up%20for%20grabs-blue)](../../issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)

Want your dev board to work with **LucidConsole**? I'll mentor you through it. **First-timers welcome.** Your name lands in the table, your board gets support, and the embedded community grows stronger.

### Why One Board Per Person?

I have a **stack of ESP8266 boards** on my bench, but I'm **deliberately not porting them myself**. This project exists to:

1. **Teach open source contribution** - Your guided first PR experience
2. **Learn embedded development** - Real hardware, real challenges, real mentorship
3. **Build community** - Each contributor becomes an expert on their board

**I could port 10 boards in a weekend, but then nobody learns anything.** Instead, I'd rather mentor 10 people through 10 boards and build a community of embedded developers.

### Perfect for You If:
- **Never contributed to open source** - I'll guide you through everything
- **New to embedded development** - Real hardware experience with safety net
- **Want to learn, not just collect** - Pick one board, master it, help others

### Not Looking For:
- **Board collectors** - "I have 40 boards, let me port them all"
- **Silent contributors** - Collaboration and questions encouraged
- **Drive-by PRs** - We're building community, not just code
- **Research-only ports** - Must own physical hardware for testing

### For Seniors:
**Experienced developers:** Please **review/test PRs and mentor newcomers** rather than submitting board ports yourself. Your experience is more valuable teaching than coding.

### Quick Start
1. **Check** [SUPPORTED_BOARDS.md](SUPPORTED_BOARDS.md) for available boards
2. **Open/comment** on "Add support: [Board Name]" issue to claim it
3. **Read** [CONTRIBUTING.md](CONTRIBUTING.md) - especially the ESP8266 journey section
4. **Study** [HARDWARE_PORTING.md](HARDWARE_PORTING.md) for technical details
5. **Start coding** - I'll mentor you through any problems

**Ready to claim a board?** [Open an issue](../../issues/new/choose) and let's get started! 🚀

---

## 🚀 Contributing to the Mission

**Before you start:** Read [CONTRIBUTING.md](CONTRIBUTING.md) to understand:
- Why this was backported from STM32 → ESP32 → ESP8266
- Why constraints teach more than abundance
- Why one board per person matters
- The journey forward (and why AI can't do it for you)

Found a bug? Want to add support for your ship's exotic hardware? The ancient development protocols still apply:

1. **Fork the repository** (that's git, it survived the quantum wars)
2. **Create your feature branch** (`git checkout -b feature/hyperdrive-support`)
3. **Commit your changes** (`git commit -am 'Add hyperdrive monitoring interface'`)
4. **Push to the branch** (`git push origin feature/hyperdrive-support`)
5. **Create a Pull Request** (and wait for code review from the ghost programmers)

---

## 📋 Archaeological License

MIT License - because even in the quantum age, open source software should be free.

See [LICENSE](LICENSE) file for full legal implications across spacetime.

---

## 🎖️ Mission Credits

Built with electrons and determination by a talking cat who believed that hardware should just work, even 129 years later.

Special thanks to:
- The ESP8266 development team (wherever you are in 2154)
- Every embedded systems engineer who chose reliability over cleverness
- The concept of "good enough" engineering that keeps spacecraft running
- Seuros, for the belly rubs during long debugging sessions and for having a router that needed fixing
- The router itself, for being the perfect napping spot that inspired this entire project
- Coffee (still the universal programming fuel - though I preferred milk)

**Remember: In space, nobody can hear your WiFi timeout. But with LucidConsole, they won't need to.**
