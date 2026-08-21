---
license: gpl-3.0
---
# FrameMeld

RIFE video frame interpolation with NVENC encoding (AV1 / HEVC / H.264), wrapped in a simple desktop GUI.

FrameMeld reuses Flowframes' RIFE CUDA implementation for interpolation, then encodes the result straight to AV1/HEVC/H.264 using NVIDIA NVENC via FFmpeg — no manual frame extraction, no juggling separate tools.

> **Nvidia GPU required.** Interpolation currently runs on RIFE CUDA, and encoding uses NVENC — both are Nvidia-only. AMD/Intel support (via RIFE-NCNN + AMF/QSV) is on the roadmap.

## Features

- RIFE CUDA interpolation (2x / 4x)
- Encode to AV1 NVENC, HEVC NVENC, or H.264 NVENC
- Configurable preset & CQ
- Auto video info preview (resolution, framerate, frame count, duration) on input selection
- Auto-generated output filename (`{name} {fps}fps.{ext}`) — just pick a folder and format
- Clean, filtered log output (no raw command dumps or FFmpeg banner spam)
- First-launch setup downloads the required runtime (FFmpeg + Python + RIFE model) automatically — no separate installs needed
- Temporary working files are cleaned up automatically after a run finishes or is cancelled

## Requirements

- **An NVIDIA GPU** with NVENC support for your target codec (AV1 NVENC requires RTX 40-series or newer; HEVC/H.264 NVENC support goes back further — check [NVIDIA's encoder support matrix](https://developer.nvidia.com/video-encode-and-decode-gpu-support-matrix-new)).
- **Internet connection on first launch**, to download the runtime (see [How the runtime works](#how-the-runtime-works) below).

## 🚀 Download FrameMeld

> ### Ready-to-use Windows build
> **No Python, FFmpeg, or RIFE installation required.**
>
> Download FrameMeld from either of the links below:

### 🟦 GitHub Releases

**[⬇️ Download the latest FrameMeld release](https://github.com/Luxbane/FrameMeld/releases/latest)**

Download the latest Windows build directly from GitHub Releases.

### 🟪 itch.io

**[⬇️ Download FrameMeld from itch.io](https://luxbane.itch.io/framemeld)**

Recommended if you prefer downloading the application through itch.io.

## How the runtime works

FrameMeld itself is a small download. The heavy dependencies — FFmpeg, a Python distribution with PyTorch/CUDA, and the RIFE model — are fetched on first launch and stored in:
```
%LOCALAPPDATA%\FrameMeld\runtime\
```
This keeps the initial download small and lets FrameMeld update those components independently of the app itself. The app won't let you start a job until FFmpeg, the Python runtime, and an AI model are all downloaded.

These files are mirrored (as `.7z` archives) from their original sources on [Hugging Face](https://huggingface.co/Luxbane/FrameMeld/tree/main) — see [Credits & Licenses](#credits--licenses) for where each component actually comes from.

## Building from source

Only needed if you want to modify FrameMeld, verify the build, or produce your own release. If you just want to use the app, use the [Download](https://luxbane.itch.io/framemeld) section above instead.

Requirements for building:

1. **Python 3.11+** available as `py` on your system PATH.

That's the only build-time requirement — building FrameMeld just compiles the GUI itself with PyInstaller. It does **not** need FFmpeg downloaded locally; those are fetched by the app at runtime (see [How the runtime works](#how-the-runtime-works)), not baked in at build time.

Steps:

1. Clone this repo.
2. Run `Build_FrameMeld.bat`.

The script will:
- Create a virtual environment, install PySide6 + PyInstaller
- Build `dist/FrameMeld/FrameMeld.exe`, bundling in `app/tools/7za.exe` (used later to extract the downloaded runtime archives)

The built app in `dist/FrameMeld/` is ready to run as-is — launch `FrameMeld.exe` and it'll prompt for the runtime download on first run, same as a downloaded release. This `dist/FrameMeld/` folder is also what gets zipped and published to Releases/itch.io.

## Repository contents

This repo only tracks source code — no bundled runtime binaries or models:

```
app/FrameMeld.py       source code
app/tools/7za.exe       bundled archive tool (used to extract the downloaded runtime)
Build_FrameMeld.bat    build script
README.md, LICENSE, THIRD_PARTY_LICENSES.md
```

`dist/`, `build/`, and `.venv/` are generated locally when you build and are not committed — excluded via `.gitignore`. `%LOCALAPPDATA%\FrameMeld\runtime` (the downloaded FFmpeg/Python/RIFE files) lives outside the repo entirely, on the end user's machine.

## Usage

1. Launch `FrameMeld.exe`. On first run, download FFmpeg, the Python runtime, and an AI model from the **Setup** section at the top.
2. Click **Input** and select a video. Its resolution, framerate, frame count, and duration will appear automatically.
3. Click **Output Folder** and pick where the result should be saved.
4. Choose an output **Format** (.mkv / .mp4 / .mov / .webm).
5. Set **Multiplier** (2x/4x), **Scale**, **Encoder**, **Preset**, and **CQ** as needed.
6. Click **START**. The output filename is generated automatically as `{input name} {new fps}fps.{format}`.

## Roadmap

- [ ] RIFE-NCNN (Vulkan) support for AMD/Intel GPUs
- [ ] Non-Nvidia encoder options (AMF, QSV, software encoders)
- [ ] Additional interpolation models (DAIN, FLAVR)
- [ ] UI for switching/re-downloading AI models after initial setup
- [ ] Optional settings panel for advanced/runtime overrides

## Credits & Licenses

FrameMeld is licensed under **GPL-3.0** (see [`LICENSE`](https://github.com/Luxbane/FrameMeld?tab=GPL-3.0-1-ov-file)), as it builds on GPL-3.0-licensed code from Flowframes.

See [`THIRD_PARTY_LICENSES.md`](./THIRD_PARTY_LICENSES.md) for full attribution, including:
- **RIFE** (MIT) — [hzwer/Practical-RIFE](https://github.com/hzwer/Practical-RIFE)
- **Flowframes** (GPL-3.0) — [n00mkrad/flowframes](https://github.com/n00mkrad/flowframes)
- **FFmpeg** (GPL/LGPL) — [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds)
- **7-Zip** (LGPL) — [7-zip.org](https://www.7-zip.org/)

This project is not affiliated with or endorsed by Flowframes, RIFE's authors, FFmpeg, or 7-Zip.
