#!/bin/bash
# Apply monkey patches to submodules after checkout

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
PATCHES_DIR="$PROJECT_ROOT/components/patches"

echo "Applying component patches..."

# Check if ssd1306 submodule exists and apply patch
if [ -d "$PROJECT_ROOT/components/ssd1306" ]; then
    if [ -f "$PATCHES_DIR/ssd1306_fix.patch" ]; then
        echo "Patching ssd1306 for ESP8266 compatibility..."
        cd "$PROJECT_ROOT/components/ssd1306"
        
        # Check if patch is already applied
        if ! grep -q "0x7f" ssd1306/ssd1306.c; then
            patch -p1 < "$PATCHES_DIR/ssd1306_fix.patch"
            echo "✓ SSD1306 patch applied"
        else
            echo "✓ SSD1306 patch already applied"
        fi
    fi
fi

echo "All patches applied successfully"