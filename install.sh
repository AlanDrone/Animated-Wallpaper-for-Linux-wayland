#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=== Cosmic Wallpaper Installer (Pop!_OS COSMIC) ==="
echo ""

echo "[1/3] Compiling native graphics engine (cosmic-wallpaper-engine)..."
make all

echo "[2/3] Installing binaries to ~/.local/bin..."
make install

echo "[3/3] Verifying environment..."
if ! echo "$PATH" | grep -q "$HOME/.local/bin"; then
    echo "Notice: ~/.local/bin is not currently in your PATH."
    echo "Add the following to your ~/.bashrc: export PATH=\"\$HOME/.local/bin:\$PATH\""
fi

echo ""
echo "✓ Installation complete!"
echo ""
echo "How to use:"
echo "  • Open the Graphical Interface (Recommended):"
echo "    Search for 'Cosmic Wallpaper' in your application menu, or run:"
echo "    cosmic-wallpaper-ui"
echo ""
echo "  • Or launch directly from the project folder:"
echo "    ./run.sh"
echo ""
echo "  • For CLI users:"
echo "    cosmic-wallpaper-client set /path/to/video.mp4"
echo "    cosmic-wallpaper-client pause | resume | toggle | status | stop"
echo ""
