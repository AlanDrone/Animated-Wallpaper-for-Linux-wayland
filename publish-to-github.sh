#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=== Cosmic Wallpaper — GitHub Publisher ==="
echo ""

# 1. Check that git is installed
if ! command -v git &> /dev/null; then
    echo "Error: git was not found on this system."
    exit 1
fi

# 2. Configure git author identity if not set
CURRENT_NAME="$(git config user.name || true)"
if [ -z "$CURRENT_NAME" ]; then
    read -p "Enter your GitHub Name or Username: " GIT_NAME
    git config user.name "$GIT_NAME"
fi

CURRENT_EMAIL="$(git config user.email || true)"
if [ -z "$CURRENT_EMAIL" ]; then
    read -p "Enter your GitHub Email: " GIT_EMAIL
    git config user.email "$GIT_EMAIL"
fi

echo ""
echo "[1/3] Staging files and creating initial commit..."
git branch -M main
git add .
git commit -m "feat: initial release of cosmic-wallpaper (GUI, daemon, native Wayland engine)" || echo "Files already committed."

echo ""
echo "[2/3] Connecting to remote repository..."
echo "If you haven't created the repository yet, visit: https://github.com/new"
echo ""
read -p "Paste the repository URL here (e.g. https://github.com/AlanDrone/Animated-Wallpaper-for-Linux-wayland.git): " REPO_URL

# Basic validation: must start with http or git@
if [ -z "$REPO_URL" ]; then
    echo "No URL provided. Local commit was saved successfully."
    echo "Whenever you're ready to push, run:"
    echo "  git remote add origin <URL>"
    echo "  git push -u origin main"
    exit 0
fi

if [[ ! "$REPO_URL" =~ ^(https?://|git@) ]]; then
    echo "Error: Invalid repository URL. It must start with 'https://' or 'git@'."
    exit 1
fi

git remote remove origin 2>/dev/null || true
git remote add origin "$REPO_URL"

echo ""
echo "[3/3] Pushing to GitHub..."
git push -u origin main

echo ""
echo "🎉 Success! Your project is now live on GitHub."
echo "Repository: ${REPO_URL%.git}"
echo ""
