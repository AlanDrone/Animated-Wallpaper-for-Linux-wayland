# Animated Wallpaper for Linux Wayland

<p align="center">
  <img src="https://raw.githubusercontent.com/AlanDrone/Animated-Wallpaper-for-Linux-wayland/main/screenshot.png" alt="Animated Wallpaper for Linux Wayland" width="800"/>
</p>

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/Platform-Linux%20Wayland-orange.svg" alt="Platform">
  <img src="https://img.shields.io/badge/Desktop-COSMIC%20Desktop-purple.svg" alt="Desktop">
  <img src="https://img.shields.io/badge/GPU-NVIDIA%20%7C%20AMD%20%7C%20Intel-green.svg" alt="GPU Support">
</p>

> **Plug-and-play animated video and GIF wallpaper engine for COSMIC Desktop with NVIDIA Wayland support.**

Supports **videos** (MP4, WebM, MKV, AVI, MOV), **animated GIFs**, and **static images** (PNG, JPG, WEBP).  
Built from scratch with a native C Wayland engine (`wlr-layer-shell`), a lightweight Python supervisor daemon, and a modern GTK4 / Libadwaita user interface. **No web engine, no Electron, no background browser, and no X11 required.**

---

## ⚡ Quick Start (TL;DR)

Open your terminal and run:

```bash
# 1. Install dependencies (Ubuntu / Pop!_OS)
sudo apt update && sudo apt install -y build-essential libwayland-dev libwayland-egl1 \
    libegl1 libgl1 libmpv2 ffmpeg python3-gi gir1.2-adw-1 python3-pil

# 2. Clone the repository
git clone https://github.com/AlanDrone/Animated-Wallpaper-for-Linux-wayland.git
cd Animated-Wallpaper-for-Linux-wayland

# 3. Launch directly (auto-compiles on first run!)
./run.sh
```

---

## ✨ Features

- 🎬 **Animated Videos** — Smooth 60 FPS playback for `.mp4`, `.webm`, `.mkv`, `.avi`, and `.mov`.
- 🎞 **Animated GIFs** — Native looping GIF playback via mpv without freezing.
- 🖼 **Static Images** — Supports high-resolution `.png`, `.jpg`, and `.webp` with zero flickering.
- 🖥 **Native Wayland Layer** — Uses `wlr-layer-shell-unstable-v1` at the background layer (`ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND`), fully integrated with `cosmic-comp`.
- 🎨 **Modern GTK4 / Libadwaita Interface**:
  - Drag-and-drop media directly onto the window.
  - Automatic thumbnail generation (using Pillow and ffmpeg).
  - One-click monitor and scaling selector.
  - Play, pause, and stop controls.
- 📐 **3 Scaling Modes (Live Switchable)**:
  - **Fit** (default): Preserves aspect ratio with letterboxing if needed. No distortion or unwanted zoom.
  - **Fill**: Pan-and-scan zoom to completely fill the screen without black bars.
  - **Stretch**: Stretches video to all 4 screen edges.
- 🛡️ **NVIDIA 4K Hardware Fix**:
  - Prevents `GL_INVALID_OPERATION` crashes on 1080p outputs (e.g. DVI-D-1 / HDMI) when loading 4K media by leveraging mpv's native aspect and zoom properties rather than ffmpeg software filter conversions.
- 🔇 **Zero Audio Overhead** — Audio playback is permanently muted; runs silently in the background.
- 🔄 **State Persistence** — Remembers your last active wallpaper, playback state, and settings across system reboots.
- 🚀 **Autostart Support** — Simple one-click switch in the UI to start with your session.

---

## 📦 Requirements & Dependencies

Make sure you have the required runtime and build packages installed.

### Pop!_OS 24.04 / Ubuntu 24.04+
```bash
sudo apt update
sudo apt install -y \
    build-essential \
    libwayland-dev \
    libwayland-egl1 \
    libegl1 \
    libgl1 \
    libmpv2 \
    ffmpeg \
    python3 \
    python3-gi \
    gir1.2-adw-1 \
    gir1.2-gtk-4.0 \
    python3-pil
```

### Fedora 40+ (COSMIC Spin / Rawhide)
```bash
sudo dnf install -y \
    gcc \
    make \
    wayland-devel \
    mesa-libEGL-devel \
    mesa-libGL-devel \
    mpv-libs \
    ffmpeg \
    python3-gobject \
    libadwaita \
    gtk4 \
    python3-pillow
```

### Arch Linux / EndeavourOS
```bash
sudo pacman -S --needed \
    base-devel \
    wayland \
    mpv \
    ffmpeg \
    python-gobject \
    libadwaita \
    gtk4 \
    python-pillow
```

---

## 🚀 Installation

### Option A: Portable / Plug-and-Play (Recommended for quick use)
You do **not** need to install anything to system directories. Just run:

```bash
git clone https://github.com/AlanDrone/Animated-Wallpaper-for-Linux-wayland.git
cd Animated-Wallpaper-for-Linux-wayland
./run.sh
```
> `./run.sh` will automatically compile `bin/cosmic-wallpaper-engine` if it is not already built and launch the GUI immediately.

---

### Option B: System-Wide User Installation (App Menu Integration)
To have the application appear in your COSMIC / Pop!_OS Application Launcher:

```bash
git clone https://github.com/AlanDrone/Animated-Wallpaper-for-Linux-wayland.git
cd Animated-Wallpaper-for-Linux-wayland
./install.sh
```

What `install.sh` does:
1. Compiles the native C engine.
2. Copies binaries (`cosmic-wallpaper-ui`, `cosmic-wallpaper-daemon`, `cosmic-wallpaper-client`, `cosmic-wallpaper-engine`) to `~/.local/bin/`.
3. Creates an application launcher desktop entry in `~/.local/share/applications/`.
4. Sets up optional systemd user service unit.

*No `sudo` is required because it installs strictly into your user's home directory.*

> **Note:** If `~/.local/bin` is not in your `$PATH`, add this line to `~/.bashrc`:
> ```bash
> export PATH="$HOME/.local/bin:$PATH"
> ```

---

## 🖥️ How to Use

### 1. Graphical Interface (GUI)
- Launch **Cosmic Wallpaper** from your app menu or run `cosmic-wallpaper-ui` (or `./run.sh`).
- **Add Media**:
  - Drag and drop any video, GIF, or picture directly into the gallery window.
  - Or click the **"Add Media"** button to browse files.
- **Select Display & Scaling**:
  - Choose your target monitor from the dropdown (or `All Monitors (*)`).
  - Select your desired scaling mode (`Fit`, `Fill`, or `Stretch`).
- **Apply**: Click **"Set as Wallpaper"**.
- **Playback**: Use the Pause/Resume and Stop buttons directly on the top bar.
- **Autostart**: Toggle the **"Start with system"** switch on the toolbar to have your wallpaper start automatically when you log in.

### 2. Command Line Interface (CLI)
For scripts, keybindings, or terminal workflows:

```bash
# Set a video or GIF wallpaper
cosmic-wallpaper-client set ~/Videos/my-wallpaper.mp4

# Set with specific monitor and scaling
cosmic-wallpaper-client set ~/Videos/my-wallpaper.mp4 --output DVI-D-1 --scaling fill

# Control playback
cosmic-wallpaper-client pause
cosmic-wallpaper-client resume
cosmic-wallpaper-client toggle
cosmic-wallpaper-client stop

# View current wallpaper and daemon status
cosmic-wallpaper-client status

# List all detected physical displays
cosmic-wallpaper-client outputs
```

---

## 🔍 Why Was This Project Built?

On the new **Pop!_OS 24.04 COSMIC Desktop**, standard animated wallpaper solutions (such as `mpvpaper` or `swww`) frequently experience silent crashes or bugs due to:

1. **Panel Reservation Bug**: By default, `wlr-layer-shell` reserving 0 exclusive zone clips the wallpaper to `1920×1024` on a 1080p display because COSMIC reserves 56px for the top/bottom panel. This engine sets `exclusive_zone = -1` and explicit zero-offsets to ensure full-bleed coverage (`1920×1080`).
2. **NVIDIA Driver `GL_INVALID_OPERATION`**: NVIDIA's Wayland driver encounters severe issues when software video filters (`vf=scale`) attempt surface format conversions on high-resolution (4K) videos on 1080p monitors. This engine eliminates software filters completely, handling pan, zoom, and scaling natively via hardware render context properties.
3. **Wayland Layer Protocol**: COSMIC does not run GNOME Shell extensions. Everything must speak native Wayland `wlr-layer-shell`.

---

## 🛠️ Architecture & Project Structure

```
Animated-Wallpaper-for-Linux-wayland/
├── src/
│   └── engine.c                        # High-performance C engine (Wayland + EGL + mpv)
├── bin/
│   ├── cosmic-wallpaper-engine         # Compiled native binary
│   ├── cosmic-wallpaper-daemon         # Background supervisor & UNIX socket IPC
│   ├── cosmic-wallpaper-client         # CLI controller
│   └── cosmic-wallpaper-ui             # GTK4 / Libadwaita graphical app
├── protocols/
│   └── wlr-layer-shell-unstable-v1.xml # Wayland layer-shell protocol specification
├── include/                            # Self-contained Khronos EGL & mpv headers
├── desktop/                            # XDG desktop entry for Application Menu
├── autostart/                          # XDG session autostart configuration
├── systemd/                            # User systemd service unit
├── Makefile                            # Clean build configuration
├── run.sh                              # Zero-configuration instant launcher
└── install.sh                          # Non-root user installer
```

---

## ❓ Frequently Asked Questions (FAQ)

<details>
<summary><b>Does this use a lot of CPU or GPU?</b></summary>
No. Video decoding is handled directly by <code>libmpv</code> using hardware acceleration (<code>hwdec=auto-safe</code>), and rendering is direct-to-EGL on Wayland with no intermediate browser or compositor overhead.
</details>

<details>
<summary><b>Where are my wallpapers and settings saved?</b></summary>
Your library list and configuration are stored cleanly in your user directory:
<ul>
  <li>Settings: <code>~/.config/cosmic-wallpaper/config.json</code></li>
  <li>Gallery library: <code>~/.config/cosmic-wallpaper/library.json</code></li>
  <li>Thumbnails cache: <code>~/.cache/cosmic-wallpaper/thumbs/</code></li>
</ul>
</details>

<details>
<summary><b>Can I close the UI while the wallpaper is playing?</b></summary>
Yes! The user interface communicates with a lightweight background daemon (<code>cosmic-wallpaper-daemon</code>). You can safely close the window anytime; your wallpaper will continue playing uninterrupted.
</details>

---

## 🗑️ Uninstallation

If you ever wish to remove the application from your system:

```bash
cd Animated-Wallpaper-for-Linux-wayland
make uninstall
```
This removes the binaries from `~/.local/bin/`, the application shortcut, and the autostart entry.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).
