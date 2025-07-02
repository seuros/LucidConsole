---
name: "Add support for <BoardName>"
about: "Port LucidConsole to a new board — mentored and great for first-time contributors."
title: "Add support: <BoardName>"
labels: ["help wanted", "good first issue", "hardware", "mentored"]
assignees: []
---

## ⚠️ Hardware Requirement
**You MUST own this board physically.** Research-only contributions are not accepted. We need real testing with real hardware.

## Board Information
- **Board name**: 
- **Datasheet/pinout**: (link)
- **Product page**: (link)
- **Where to buy**: (link)
- **Estimated price**: $

## Goal
Bring up LucidConsole on **<BoardName>** with actual hardware testing:
- ✅ UART RX working (verified with real device)
- ✅ **Stealth UART** (TX tri-stated until user enables - critical!)
- ✅ WiFi AP mode functional at http://10.10.10.1
- ✅ Live serial monitoring through web interface
- ✅ Bidirectional UART communication verified
- ⬜ (Optional) Buttons/LED/OLED mapped and working

## Checklist (All Required)
- [ ] **I own this board physically** ✋
- [ ] Fork repo and create `boards/<board_name>/board.h`
- [ ] Map pins based on actual board, not just datasheet research
- [ ] Build and flash firmware to real hardware
- [ ] **Photo proof**: Board connected to target device
- [ ] **Boot test**: Verify target device boots normally with LucidConsole attached
- [ ] **Stealth UART test**: Confirm TX disabled doesn't interfere with target boot
- [ ] **Web UI test**: Screenshot of interface at http://10.10.10.1
- [ ] **UART bridge test**: Prove bidirectional communication works
- [ ] **Serial log**: Capture boot output showing all systems working
- [ ] Open PR with all evidence above

## Hardware Testing Evidence (Required for PR)
**Your PR will be rejected without these:**

1. **Connection photo** - Your board wired to a target device
2. **Serial console output** - Boot log showing WiFi, HTTP server, UART working
3. **Web interface screenshot** - Proving you can access http://10.10.10.1
4. **UART test proof** - Bidirectional communication working
5. **Target device proof** - Show target boots normally with LucidConsole connected

## Notes / Board Quirks
(Document any special requirements, inverted signals, power quirks, boot pin issues)

## For Senior Contributors
**Experienced developers:** Please **review and mentor** rather than submitting board PRs yourself. Your experience is more valuable guiding newcomers than adding another port.

## Claim This Board
Comment **"I'm taking this"** to claim. Must include:
- Confirmation you own the physical hardware
- Estimated timeline for testing and PR submission
- Any questions about the porting process

**I'll mentor you through tri-state timing, pin conflicts, and debugging. First-time contributors especially welcome!**