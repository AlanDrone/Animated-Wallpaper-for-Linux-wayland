<p align="center">
  <h1>Animated Wallpaper for Linux Wayland</h1>
</p>

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-Free%20for%20Use%20%7C%20Attribution%20Required-blue.svg" alt="License: Free & Open Use with Attribution"></a>
  <img src="https://img.shields.io/badge/Platform-Linux%20%7C%20Wayland-orange.svg" alt="Platform: Linux Wayland">
  <img src="https://img.shields.io/badge/Desktop-COSMIC%20Desktop-blueviolet.svg" alt="COSMIC Desktop">
  <img src="https://img.shields.io/badge/GPU-NVIDIA%20%7C%20AMD%20%7C%20Intel-green.svg" alt="GPU Support">
  <img src="https://img.shields.io/badge/Built%20With-C%20%7C%20Python%20%7C%20GTK4-informational.svg" alt="Tech Stack">
</p>

<p align="center">
  <strong>Plug-and-play animated video and GIF wallpaper engine for COSMIC Desktop with NVIDIA Wayland support.</strong><br>
  Supports videos (MP4, WebM, MKV, AVI, MOV), animated GIFs, and static images (PNG, JPG, WEBP).<br>
  <em>No Electron. No browser. No X11. No root required.</em>
</p>

---

## Table of Contents

- [Why This Exists](#why-this-exists)
- [Features](#features)
- [Requirements](#requirements)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [How to Use](#how-to-use)
- [Scaling Modes](#scaling-modes)
- [Project Structure](#project-structure)
- [Technical Details](#technical-details)
- [FAQ](#faq)
- [Uninstall](#uninstall)
- [License](#license)

---

## Why This Exists

On **Pop!_OS 24.04 with COSMIC Desktop**, popular animated wallpaper tools such as `mpvpaper` and `swww` fail silently or crash. This project was built to solve three specific issues:

| # | Problem | Root Cause | Fix Applied |
|---|---------|-----------|-------------|
| 1 | Wallpaper renders at `1920×1024` instead of `1920×1080` | COSMIC's panel reserves 56 px by default; `wlr-layer-shell` clips the surface | Sets `exclusive_zone = -1` + `set_size(0, 0)` for full-bleed coverage |
| 2 | `GL_INVALID_OPERATION` crash on NVIDIA with 4K video on a 1080p output | `vf=scale` software filters cannot convert NVIDIA hardware-decoded CUDA frames to a CPU-side GL-compatible format | Eliminates all software filters; uses mpv native render properties (`panscan`, `keepaspect`) entirely on the GPU |
| 3 | GNOME Shell extensions incompatible with COSMIC | COSMIC does not run GNOME Shell — extension APIs crash at startup | Implements `wlr-layer-shell-unstable-v1` Wayland protocol directly in C |

---

## Features

- **Video wallpapers** — smooth looping for `.mp4`, `.webm`, `.mkv`, `.avi`, `.mov`
- **Animated GIFs** — native looping via mpv, no freezing or flickering
- **Static images** — infinite display for `.png`, `.jpg`, `.webp` without blinking
- **Native Wayland layer** — uses `wlr-layer-shell-unstable-v1` at `BACKGROUND` layer depth, compatible with `cosmic-comp`
- **Modern GTK4 / Libadwaita GUI** — drag-and-drop, live thumbnail gallery, adapts to system dark/light theme
- **3 live-switchable scaling modes** — change between Fit, Fill, and Stretch without restarting
- **Hardware-accelerated decoding** — `hwdec=auto-safe` via mpv; works transparently with NVIDIA, AMD, and Intel
- **Silent background** — audio is permanently muted; zero interference with system audio
- **Auto-restore** — last active wallpaper is reapplied automatically on login
- **Persistent library** — gallery survives reboots (`~/.config/cosmic-wallpaper/library.json`)
- **One-click autostart** — toggle session startup directly from the GUI toolbar
- **Multi-monitor support** — target a specific output (e.g. `HDMI-A-1`, `DP-1`, `eDP-1`) or all monitors at once
- **Full CLI control** — set, pause, resume, toggle, stop, and query status from a terminal

---

## Requirements

The following packages must be installed before building or running the project.

### Pop!_OS 24.04 / Ubuntu 24.04 (Noble) or later

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
    gir1.2-gtk-4.0 \
    gir1.2-adw-1 \
    python3-pil
```

### Fedora 40+ / COSMIC Spin

```bash
sudo dnf install -y \
    gcc make \
    wayland-devel \
    mesa-libEGL \
    mesa-libGL \
    mpv-libs \
    ffmpeg \
    python3-gobject \
    gtk4 \
    libadwaita \
    python3-pillow
```

### Arch Linux / EndeavourOS / Manjaro

```bash
sudo pacman -S --needed \
    base-devel \
    wayland \
    mesa \
    mpv \
    ffmpeg \
    python-gobject \
    gtk4 \
    libadwaita \
    python-pillow
```

> **Note:** `libmpv-dev` / `libmpv-devel` (header-only dev packages) are **not** required. This project bundles the necessary public headers in `include/mpv/` and `include/EGL/`, so only the runtime library (`libmpv2`) needs to be installed.

---

## Quick Start

Three steps and the GUI is open:

```bash
# 1. Install dependencies (Pop!_OS / Ubuntu)
sudo apt update && sudo apt install -y \
    build-essential libwayland-dev libwayland-egl1 libegl1 libgl1 \
    libmpv2 ffmpeg python3-gi gir1.2-gtk-4.0 gir1.2-adw-1 python3-pil

# 2. Clone the repository
git clone https://github.com/AlanDrone/Animated-Wallpaper-for-Linux-wayland.git
cd Animated-Wallpaper-for-Linux-wayland

# 3. Launch — auto-compiles the engine on the first run
./run.sh
```

---

## Installation

### Option A — Portable (no system install required)

```bash
git clone https://github.com/AlanDrone/Animated-Wallpaper-for-Linux-wayland.git
cd Animated-Wallpaper-for-Linux-wayland
./run.sh
```

`run.sh` automatically compiles `bin/cosmic-wallpaper-engine` if it has not been built yet, then launches the GUI. The project can be run from any location — nothing is installed to the system.

### Option B — User Install (app appears in the COSMIC application menu)

```bash
git clone https://github.com/AlanDrone/Animated-Wallpaper-for-Linux-wayland.git
cd Animated-Wallpaper-for-Linux-wayland
./install.sh
```

What `install.sh` does:

1. Compiles the native C engine (`make all`)
2. Copies all four binaries to `~/.local/bin/`
3. Creates a desktop launcher at `~/.local/share/applications/`
4. Installs an optional systemd user service unit

> **No `sudo` is needed.** Everything is installed inside your home directory (`~/.local/`).

After installation, if the `cosmic-wallpaper-ui` command is not found, add `~/.local/bin` to your PATH:

```bash
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

---

## How to Use

### Graphical Interface

1. Open **Cosmic Wallpaper** from your app menu, or run `./run.sh` from the project folder.
2. **Add your media** in one of two ways:
   - **Drag and drop** any video, GIF, or image file directly into the gallery window.
   - Click the **"Add Media"** button in the header bar to browse for files.
3. **Configure display settings** using the toolbar:
   - **Monitor** — select a specific output (e.g. `HDMI-A-1`, `DP-1`) or `All Monitors (*)`.
   - **Scaling** — choose `Fit`, `Fill`, or `Stretch` (see [Scaling Modes](#scaling-modes) below).
4. Click **"Set as Wallpaper"** on any gallery card.
5. Use the **Pause/Resume** (⏸) and **Stop** (⏹) buttons in the header to control playback.
6. Toggle **"Start with system"** on the toolbar to enable or disable autostart at login.

> **Closing the window does not stop the wallpaper.** The background daemon (`cosmic-wallpaper-daemon`) keeps running independently. Use the Stop button or the CLI to terminate playback.

### Command Line Interface

```bash
# Set a wallpaper (auto-detect monitor, fit scaling)
cosmic-wallpaper-client set ~/Videos/my-wallpaper.mp4

# Set with explicit monitor and scaling mode
cosmic-wallpaper-client set ~/Videos/my-wallpaper.mp4 --output HDMI-A-1 --scaling fill

# Playback control
cosmic-wallpaper-client pause
cosmic-wallpaper-client resume
cosmic-wallpaper-client toggle       # toggles between pause and play

# Stop the wallpaper completely
cosmic-wallpaper-client stop

# Show current daemon status
cosmic-wallpaper-client status

# List all detected physical outputs
cosmic-wallpaper-client outputs
```

---

## Scaling Modes

| Mode | Behavior | Best For |
|------|----------|----------|
| `fit` *(default)* | Preserves aspect ratio. Adds letterboxing (black bars) if ratios differ. No distortion, no cropping. | Photography, artwork, images where nothing should be lost |
| `fill` | Pan-and-scan zoom: fills the screen completely, cropping edges to maintain aspect ratio. No black bars. | Cinematic videos and wallpapers made for full-screen display |
| `stretch` | Stretches to cover all four edges, ignoring aspect ratio. May cause distortion. | Short abstract loops where distortion is acceptable |

Scaling mode can be changed **live** from the dropdown — the wallpaper continues playing without restarting.

---

## Project Structure

```
Animated-Wallpaper-for-Linux-wayland/
│
├── src/
│   └── engine.c                    # Native C rendering engine (Wayland + EGL + libmpv)
│
├── bin/
│   ├── cosmic-wallpaper-engine     # Compiled C binary (generated by `make`)
│   ├── cosmic-wallpaper-daemon     # Python background supervisor + UNIX socket IPC
│   ├── cosmic-wallpaper-client     # Python CLI controller
│   └── cosmic-wallpaper-ui         # Python GTK4 / Libadwaita GUI application
│
├── protocols/
│   └── wlr-layer-shell-unstable-v1.xml    # Wayland protocol definition
│
├── include/
│   ├── EGL/                        # Bundled Khronos EGL headers
│   ├── KHR/                        # Bundled Khronos KHR platform headers
│   └── mpv/                        # Bundled libmpv public API headers
│
├── autostart/
│   └── cosmic-wallpaper.desktop    # XDG session autostart entry
│
├── desktop/
│   └── io.github.animated_wallpaper.desktop    # Application menu launcher
│
├── systemd/
│   └── cosmic-wallpaper.service    # Optional systemd user service unit
│
├── Makefile                         # Build configuration
├── run.sh                           # Portable launcher (compiles on first run + starts UI)
├── install.sh                       # User-local installer (no root required)
├── LICENSE                          # MIT License
└── README.md
```

---

## Technical Details

| Component | Technology | Notes |
|-----------|------------|-------|
| Wayland layer | `wlr-layer-shell-unstable-v1` | Anchored to all edges at `BACKGROUND` depth; `exclusive_zone = -1` for full-screen coverage including behind panels |
| Rendering | EGL + OpenGL via libmpv | `wl_egl_window` surface; mpv render context drives GL frame delivery |
| Video decode | libmpv with `hwdec=auto-safe` | Hardware-accelerated on NVIDIA, AMD, and Intel; no software decoder pipeline |
| Scaling | mpv native render properties | `panscan`, `keepaspect` — eliminates `vf=scale` software filters that cause NVIDIA GL surface errors |
| Frame sync | `display-resample` | `video-sync=display-resample` matches video to display refresh to prevent tearing |
| Static images | `image-display-duration=inf` | Holds the decoded frame indefinitely without blinking or polling |
| IPC | UNIX Domain Socket | JSON protocol over `$XDG_RUNTIME_DIR/cosmic-wallpaper.sock` |
| 4K handling | ffprobe + mpv native downscaling | Detects resolution before engine launch; mpv handles size reduction on the GPU |

---

## FAQ

<details>
<summary><b>Does this work with AMD or Intel GPUs?</b></summary>
<br>
Yes. The engine sets <code>hwdec=auto-safe</code>, which selects the best available hardware decoder automatically. The project was developed and tested on modern NVIDIA GPUs on Wayland, but the architecture is GPU-agnostic and works seamlessly with AMD and Intel graphics.
</details>

<details>
<summary><b>How much CPU or memory does it use?</b></summary>
<br>
Very little. Video decoding is handled entirely by <code>libmpv</code> using GPU hardware acceleration. The Python daemon is idle between IPC commands and the C engine is event-driven. A typical 1080p MP4 wallpaper uses under 2% CPU on modern hardware.
</details>

<details>
<summary><b>Can I close the GUI window while the wallpaper is playing?</b></summary>
<br>
Yes. Closing the window only hides the interface. The background daemon and engine process continue running independently. Use the Stop button (or <code>cosmic-wallpaper-client stop</code>) if you want to fully terminate playback.
</details>

<details>
<summary><b>Where are my settings and library saved?</b></summary>
<br>
Everything is written to your user home directory — nothing outside of <code>~</code>:
<ul>
  <li>Daemon config: <code>~/.config/cosmic-wallpaper/config.json</code></li>
  <li>Media library: <code>~/.config/cosmic-wallpaper/library.json</code></li>
  <li>Daemon log: <code>~/.config/cosmic-wallpaper/daemon.log</code></li>
  <li>Thumbnail cache: <code>~/.cache/cosmic-wallpaper/thumbs/</code></li>
  <li>IPC socket: <code>$XDG_RUNTIME_DIR/cosmic-wallpaper.sock</code></li>
</ul>
</details>

<details>
<summary><b>My wallpaper only covers part of the screen (e.g. 1920×1024 on a 1080p display).</b></summary>
<br>
This is a known COSMIC panel reservation behavior. The engine handles it automatically by setting <code>exclusive_zone = -1</code> on the layer surface, which instructs <code>cosmic-comp</code> to give the wallpaper the full output size regardless of panel placement. If the issue persists, please open a bug report with your compositor version.
</details>

<details>
<summary><b>My 4K video causes a crash or a black screen on a 1080p NVIDIA output.</b></summary>
<br>
This is the <code>GL_INVALID_OPERATION</code> bug caused by the NVIDIA driver rejecting software-side surface format conversions on hardware-decoded frames. This engine avoids all software video filters and uses only mpv's native GPU-side render properties (<code>panscan</code>, <code>keepaspect</code>) for scaling — which eliminates the crash entirely.
</details>

<details>
<summary><b>Does this work with multiple monitors?</b></summary>
<br>
Yes. Select a specific output name (e.g. <code>HDMI-A-1</code>, <code>DP-1</code>, <code>eDP-1</code>) from the Monitor dropdown, or choose <code>All Monitors (*)</code>. The daemon detects connected outputs via <code>/sys/class/drm</code> without relying on any compositor-specific extension.
</details>

---

## Uninstall

To remove all installed files:

```bash
cd Animated-Wallpaper-for-Linux-wayland
make uninstall
```

This removes:
- All binaries from `~/.local/bin/`
- The application launcher from `~/.local/share/applications/`
- The autostart entry from `~/.config/autostart/`
- The systemd user service from `~/.config/systemd/user/`

> Personal data — your media library, config files, and thumbnail cache under `~/.config/cosmic-wallpaper/` and `~/.cache/cosmic-wallpaper/` — is **not** deleted.

---

## License

This project is licensed under the [Animated Wallpaper License](LICENSE) (Free & Open Use with Attribution):

- **Free & Open**: Free to download, run, inspect, and modify for personal and community use.
- **Content Creation & Streaming**: You are expressly permitted to use this wallpaper in live streams (Twitch, YouTube, Kick, etc.), broadcasts, podcasts, and recorded videos — including monetized content.
- **Attribution Required (No Plagiarism)**: You may not claim you created this software, rebrand it, or remove the original copyright and author attribution notices.
- **No Resale**: Selling, charging fees for, or sublicensing this software (or modified versions) is strictly prohibited.
