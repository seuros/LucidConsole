# Component Patches (Linux Kernel Style)

This directory contains patches for third-party components, following the Linux kernel patching convention.

## Why Patches?

The upstream SSD1306 library causes **boot loops** on ESP8266 due to excessive power consumption during initialization. Our patches fix this issue while keeping the upstream repository as a clean submodule.

## Patches Applied

### 0001-ssd1306-reduce-power-prevent-brownout.patch
- **Problem**: Original init turns on ALL pixels (8192) at once + high contrast = brownout
- **Solution**: Reduce contrast from 0x9f to 0x7f, disable whole_display_lighting
- **Impact**: Eliminates boot loops on ESP8266

## How It Works (Linux Style)

1. **Submodules**: Components are git submodules (clean upstream tracking)
2. **Patches**: Stored in `components/patches/` with descriptive names
3. **Auto-apply**: Makefile applies patches before building (like Linux kernel)
4. **Idempotent**: Patches only apply if not already present

## Manual Operations

```bash
# Apply patches manually
cd components/ssd1306
patch -p1 < ../patches/0001-ssd1306-reduce-power-prevent-brownout.patch

# Create new patch (after modifying submodule)
cd components/ssd1306
git diff > ../patches/0002-your-new-fix.patch

# Using quilt (Linux kernel tool)
quilt push -a  # Apply all patches
quilt pop -a   # Remove all patches
```

## Adding New Patches

1. Modify the submodule code
2. Generate patch: `git diff > ../patches/0002-description.patch`
3. Add patch name to `series` file (if using quilt)
4. Test: `make clean && make`

This approach keeps our modifications **documented, versioned, and reversible** - exactly like the Linux kernel handles thousands of patches to upstream code.