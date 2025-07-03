#!/bin/bash
# Install git hooks for automatic patching
# Run this after cloning the repository

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Installing git hooks..."

# Create post-checkout hook
cat > "$PROJECT_ROOT/.git/hooks/post-checkout" << 'EOF'
#!/bin/bash
# Auto-apply patches after submodule update
if [ -f "Makefile" ]; then
    make patch-components 2>/dev/null || true
fi
EOF

chmod +x "$PROJECT_ROOT/.git/hooks/post-checkout"

# Create post-merge hook (for pull operations)
cp "$PROJECT_ROOT/.git/hooks/post-checkout" "$PROJECT_ROOT/.git/hooks/post-merge"

echo "✓ Git hooks installed"
echo "Patches will now auto-apply after submodule updates"