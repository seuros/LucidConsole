#!/bin/bash
# Setup submodules with patches - Linux kernel style
# Target OS: Linux/macOS (If you're going to Lucid, you better be in the dark... terminal)
# Windows users: WSL2 or cry

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
PATCHES_DIR="$PROJECT_ROOT/components/patches"

# Detect OS for terminal aesthetics
if [[ "$OSTYPE" == "darwin"* ]]; then
    OS_NAME="macOS"
    TERMINAL_MSG="Terminal.app or iTerm2 - good choices"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS_NAME="Linux"
    TERMINAL_MSG="Real terminal, real developer"
else
    echo "⚠️  Windows detected. Please use WSL2 or a real operating system."
    echo "   LucidConsole development requires a proper terminal environment."
    exit 1
fi

echo "╔══════════════════════════════════════════════════════════╗"
echo "║           LucidConsole Submodule Setup                    ║"
echo "║                                                           ║"
echo "║  'If you're going to Lucid, you better be in the dark'   ║"
echo "║                    - Terminal Required -                  ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "OS: $OS_NAME ($TERMINAL_MSG)"
echo "Setting up components with Linux-style patches..."

# Initialize and update submodules
echo "1. Fetching submodules..."
git submodule init
git submodule update

# Apply patches to each component
echo "2. Applying patches..."

# SSD1306 patches
if [ -d "$PROJECT_ROOT/components/ssd1306" ]; then
    echo "   Patching ssd1306..."
    cd "$PROJECT_ROOT/components/ssd1306"
    
    for patch in "$PATCHES_DIR"/0001-*.patch; do
        if [ -f "$patch" ]; then
            # Check if patch is already applied
            if ! grep -q "0x7f" ssd1306/ssd1306.c 2>/dev/null; then
                echo "   → Applying $(basename "$patch")"
                patch -p1 -N < "$patch" || true
            else
                echo "   → $(basename "$patch") already applied"
            fi
        fi
    done
fi

cd "$PROJECT_ROOT"

echo "3. Setup complete!"
echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║                    Setup Complete                         ║"
echo "║                                                           ║"
echo "║  Components ready with ESP8266 compatibility patches      ║"
echo "║                                                           ║"
echo "║  Next steps:                                             ║"
echo "║    $ make BOARD=ideaspark_oled_0.96_v2.1 all            ║"
echo "║    $ make flash ESPPORT=/dev/ttyUSB0                    ║"
echo "║                                                           ║"
echo "║  Remember: Real debugging happens in the terminal,       ║"
echo "║           not in some GUI pretending to be helpful.      ║"
echo "╚══════════════════════════════════════════════════════════╝"