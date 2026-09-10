#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# 1. Ensure the native engine binary is compiled
if [ ! -f "$SCRIPT_DIR/bin/cosmic-wallpaper-engine" ]; then
    echo "⚡ First run detected: compiling native graphics engine..."
    make all > /dev/null 2>&1 || make all
    echo "✓ Engine ready!"
fi

# 2. Launch the graphical user interface
exec python3 "$SCRIPT_DIR/bin/cosmic-wallpaper-ui" "$@"
