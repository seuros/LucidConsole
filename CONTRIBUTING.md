# Contributing to LucidConsole

> **"If you're going to Lucid, you better be in the dark"** - Terminal development only.

**Required Environment:** Linux or macOS with a real terminal  
**Windows users:** WSL2 or contribute to something else  
**IDE users:** `vim`, `emacs`, or `nano` - pick your weapon  

Welcome! This project is designed to teach open source contribution and embedded development. **One board per person** - let's share the learning.

## Philosophy

I have a stack of ESP8266/ESP32 boards on my bench, but I'm **intentionally not porting them all myself**. This project exists to:

1. **Teach open source contribution** - Your first PR, guided experience
2. **Learn embedded development** - Real hardware, real challenges, real solutions
3. **Build community** - Each board gets a dedicated contributor who becomes an expert

## Board Contribution Rules

### ✅ Perfect Contributor
- **One board per person** - Pick one, master it, help others with theirs
- **First-timer friendly** - Never contributed to open source? Perfect!
- **Learning focused** - Ask questions, document problems, share solutions

### ❌ Not What We Want
- **Board collectors** - "I have 40 boards, let me port them all"
- **Drive-by PRs** - Submit and disappear
- **Silent workers** - No questions, no collaboration
- **Research-only PRs** - Pinout copying without actual hardware testing

## How to Contribute

### For Junior/Learning Contributors

#### 1. Pick Your Board (One Only!)
- **Must own the physical hardware** - no research-only contributions
- Check [SUPPORTED_BOARDS.md](SUPPORTED_BOARDS.md) for what's available
- Open an issue: "Add support: [Your Board Name]"
- Comment "I'm taking this" - I'll mentor you through it

#### 2. Proof of Hardware Required
**Every PR must include:**
- **Photo of your board** connected to a target device
- **Serial console output** showing successful boot and operation
- **Web interface screenshot** demonstrating functionality
- **Test results** proving UART bridge actually works

*I could add every board by researching pinouts online, but that defeats the purpose. Real hardware testing ensures quality and teaches you embedded development.*

### For Senior Contributors

**Seniors: Please don't submit board PRs yourself.** Instead:
- **Review and test** junior contributor PRs
- **Mentor newcomers** through technical challenges  
- **Guide architecture decisions** and best practices
- **Help with debugging** when juniors get stuck

Your experience is more valuable teaching others than adding another board port.

### 3. Learning Path
- Fork the repo
- Copy an existing board definition from `boards/`
- Map your pins (I'll help with quirks)
- Test step-by-step (bootloader, WiFi, UART, web interface)
- Submit PR with photos and documentation

### 3. Mentorship Promise
- **Fast feedback** - I'll review PRs quickly and help debug
- **No stupid questions** - First time with ESP8266? Great, let's learn together
- **Real support** - Boot pin issues? Tri-state problems? I've been there

## Why One Board Per Person?

This isn't about efficiency - it's about **learning**. When you own a board port:
- You become the expert on that hardware
- You help future users with that board
- You learn the entire codebase, not just your changes
- You can mentor the next contributor

I **deliberately** limit myself to one board so there's room for you to contribute meaningfully.

## What's in it for You?

- **Your name** in the supported boards table
- **Real experience** with embedded development
- **Open source contribution** on your GitHub profile
- **Mentorship** from someone who's debugged this stuff for years
- **Community** - help others with their board ports

## Advanced Contributions Welcome

### Hardware Flow Control (RTS/CTS)
Got a board with hardware flow control pins? Add RTS/CTS support! Many industrial devices need proper flow control, especially when dealing with MAX232 level converters for true RS-232 communication. This is a great intermediate project - not too complex, but teaches you about real UART beyond just TX/RX.

**Build it, then show it off:** Once you get it working, go show Dave Plummer (Dave's Garage) how to control his PDP-11 behemoth remotely. Or Ben Eater's breadboard computers. Or any retrocomputing YouTuber with serial ports gathering dust. They love this stuff, and you might get a shoutout. Hell, they might even feature your contribution.

I'm not here to be YouTube famous - I'm here to launch a concept. This device is trivial to build but powerful in application. You want exposure? Take what you learned here and show the world what you can connect it to.

### Why I Did the Ideaspark OLED Board Myself
I already ported the `boards/ideaspark_oled_0.96_v2.1` - the one with the built-in OLED. Why? Because it's the **most deceptively complex** board for beginners:
- **Non-standard I2C pinout** - SDA/SCL aren't where ESP8266 normally puts them
- **SPI flash conflicts** - the OLED uses pins that can brick your board
- **Power sequencing matters** - OLED needs proper initialization or stays black
- **Clone chip quirks** - these SSD1306 clones need inverted bits and special handling

I didn't want someone's first contribution to be debugging why their display shows garbage. That's not learning, that's suffering.

### This is RTOS, Not Arduino
Let me be clear: **This uses ESP8266 RTOS SDK, not Arduino.**

Why does this matter? Because with RTOS you learn to:
- **Manage real tasks** - not just loop() and delay()
- **Handle interrupts properly** - not Arduino's attachInterrupt() hand-holding
- **Use real semaphores and mutexes** - actual concurrent programming
- **Control memory** - you have 80KB of RAM, use it wisely
- **Build complex systems** - WiFi + Web Server + UART + Display all running in parallel

With Arduino, you get 30KB of your binary just for the framework. With RTOS, you have space for actual features. Arduino holds your hand. RTOS teaches you to walk. When that recruiter asks "Do you know embedded RTOS?" you can say yes, not "I know Arduino."

The ESP8266 has 4MB of flash. Arduino wastes most of it. RTOS lets you use it. OTA updates, SPIFFS, multiple partition schemes - real embedded development, not toy projects.

### The ESP8266 Journey: Backward to Move Forward

LucidConsole was actually **backported** from STM32 → ESP32 → ESP8266. Yes, backward. Why?

**Because constraints teach you more than abundance.**

With STM32 or ESP32, you have luxury. Need a feature? Add 5 libraries. Want more RAM? You've got it. But ESP8266? You get:
- **80KB RAM** - not 520KB like ESP32
- **One core** - not dual-core luxury
- **$2 boards with OLED** - the MCU is basically free vs $6-9 for ESP32
- **Framework reuse mandatory** - can't just pile on libraries

When you can't just throw libraries at problems, you learn to actually solve them. When every KB matters, you learn what really matters.

This is why GTA 7 will probably need 2TB just for assets - because when you have infinite space, you stop caring about efficiency. Unless they generate it on the fly, but that would require actual engineering instead of just throwing storage at the problem.

**The goal?** Port this back to STM32 or something even better. But that journey should be an adventure others can read, learn from, and contribute to. Each port teaches something new:
- ESP8266 → STM32: Learn about hardware abstraction layers
- STM32 → RISC-V: Learn about architecture differences
- RISC-V → PIC: Learn why some platforms deserve to die

This is why I don't want **drive-by contributors** - the ones who port a board and vanish. I want people who document their journey, share their struggles, help the next person.

I could ask Grok, Gemini, and Claude to scour the planet and build me versions for every chip ever made. Hell, they'd probably even port it to a PIC16F84 just to prove they could (LOL). But what's the point? Nobody learns from AI showing off. People learn from humans struggling, solving, and sharing.

### Build on Top of This
LucidConsole is just the foundation. In my drawer, I have versions that:
- **Interrupt U-Boot sequences** - catch that 1-second window to stop autoboot
- **Auto-exploit routers** - educational purposes only, of course. My ISP in Morocco claimed that switching routers could cause fire hazards. So naturally, I rooted their router and switched to my own ONT fiber setup. No fires yet. Turns out the only thing burning was their monopoly.
- **Bridge to CAN bus** - because cars need debugging too
- **Talk to 1970s mainframes** - yes, really, via current loop

I could dump my 4 years of projects here. Post 50 boards, 200 use cases, make YouTube videos explaining every line. But then what? You'd watch, copy-paste, and learn nothing. That's not mentorship, that's entertainment.

### The Hard Truth About Learning

**"You learn like it's 1990: reading code, technical magazines, and getting burn marks from the soldering iron. Or you get replaced by AI in the next five years."**

AI can copy-paste faster than you. AI can watch YouTube tutorials at 100x speed. But AI can't:
- Debug why your board works on USB power but not battery
- Smell when a voltage regulator is about to let the magic smoke out
- Feel when a chip is running too hot
- Know that sometimes the solution is to add a 10µF capacitor "because it feels right"

The burn marks on my fingers aren't accidents - they're experience. Each one taught me something a YouTube video couldn't. When you're holding a hot iron and reading someone else's code at 2 AM trying to figure out why SPI conflicts with your debug output, you're learning in a way that sticks.

## Getting Started

1. **Read** [HARDWARE_PORTING.md](HARDWARE_PORTING.md) for technical details
2. **Choose** a board from the issues or propose a new one
3. **Comment** on the issue to claim it
4. **Start** - I'll guide you through the first steps
5. **Struggle** - That's where the learning happens
6. **Ask questions** - After you've tried to solve it yourself
7. **Success** - Your name in the repo, your knowledge permanent

Remember: **One board, one person, one great learning experience.**

### A Message to Senior Developers and Experienced Engineers

If you're experienced and like this mentorship concept, please - take it and run with it. Mentor others. Fork this approach. I don't care about the concept being copied. Spread it. Teach it. Make it better.

**Use any platform that makes sense:** Port this to STM32, implement it on an FPGA, use a RISC-V chip, whatever. The concept works anywhere that can be expanded later. Just avoid platforms that need expensive compilers (looking at you, certain FPGA vendors charging $3000 for a license) or require proprietary blobs to function. If learners need to pay hundreds just to compile their code, that's not education - it's gatekeeping.

But here's what NOT to do: Don't take this and claim you invented the teaching method, then start exploiting newcomers. Don't create a proprietary "learning platform" where people have to build around your product, creating dependencies on your "ecosystem." That's not mentorship - it's exploitation in disguise.

Unless your platform is truly free and libre (actually free, not "freemium" garbage), you're just another vendor lock-in scheme wearing a teacher's mask.

The whole point here is learning without chains. Copy the concept, not the control. Use tools that stay free, on hardware that stays hackable.

Let's build something awesome together! 🚀
