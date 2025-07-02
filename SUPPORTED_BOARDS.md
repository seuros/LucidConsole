# Supported Boards

LucidConsole is designed to be board-agnostic. Each board gets its own contributor and maintainer.

## Currently Supported

| Board | Added By | PR | Notes |
|-------|----------|----|----|
| Ideaspark OLED v2.1 | @seuros | - | Reference implementation, I2C on GPIO12/14 |
| **Your board here** | **you** | **?** | **Pick one and claim it!** |

## Boards Waiting for Adoption

I have these boards on my bench, but I'm **deliberately not porting them** - that's your job! Each one needs a dedicated contributor who will become the expert for that hardware.

**Available for claiming** (open an issue to claim one):

- [ ] **ESP-12E module** - Classic bare module, great for learning pin mapping
- [ ] **Wemos D1 Mini** - Super popular, lots of users would benefit  
- [ ] **NodeMCU v3** - Another crowd favorite
- [ ] **ESP-12F module** - Similar to 12E but newer
- [ ] **Generic USB-TTL + ESP8266** - DIY approach
- [ ] **Wemos D1 R2** - Original D1 form factor
- [ ] **SparkFun ESP8266 Thing** - Unique pin layout
- [ ] **Adafruit HUZZAH ESP8266** - Different power/pin arrangement

## Why One Board Per Person?

This isn't about speed - it's about **learning and community**:

- **You become the expert** on your board and help future users
- **Learn the entire codebase**, not just copy-paste  
- **Real mentorship** - I guide you through the tricky parts
- **Your name stays with your board** - ongoing ownership and pride

I could port all these boards myself in a weekend, but then nobody learns anything. Instead, I'd rather mentor 10 people through 10 boards and build a community of embedded developers.

## Selection Rules

- **One board per contributor** - Pick your favorite, master it, then help others
- **No hoarding** - Don't claim multiple boards  
- **First-timers welcome** - Never done open source? Perfect starting point!
- **Collaboration encouraged** - Ask questions, share problems, help each other

## Want to Claim a Board?

1. **Check the issues** for existing "Add support: BoardName" issues
2. **Open a new issue** if your board isn't listed
3. **Comment "I'm taking this"** to claim it
4. **Read [HARDWARE_PORTING.md](HARDWARE_PORTING.md)** for technical details
5. **Start coding** - I'll mentor you through any problems

## Board Requirements

Your board needs:
- **ESP8266 chip** (any variant)
- **UART pins accessible** (RX/TX)  
- **Power supply** (USB or external)
- **Programming interface** (built-in USB-TTL or external)

Nice to have but optional:
- Status LED
- Boot button  
- OLED display
- Additional GPIO breakouts

## Contributor Hall of Fame

Each successful board port gets:
- **Your name** in this table permanently
- **Shout-out** in release notes
- **Maintainer status** for your board's issues
- **Learning experience** you can't get anywhere else

Ready to claim a board? **Open an issue and let's get started!** 🚀