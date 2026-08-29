---
license: gpl-3.0
---
# FrameMeld

RIFE video frame interpolation with hardware-accelerated encoding (AV1 / HEVC / H.264), wrapped in a simple desktop GUI.

FrameMeld reuses Flowframes' RIFE implementation (CUDA or NCNN/Vulkan) for interpolation, then encodes the result straight to AV1/HEVC/H.264 using your GPU's hardware encoder via FFmpeg — no manual frame extraction, no juggling separate tools.

> **NVIDIA, AMD, and Intel GPUs are all supported.** Pick RIFE CUDA (NVIDIA only, fastest) or RIFE NCNN (NVIDIA/AMD/Intel via Vulkan) for interpolation, and NVENC/AMF/QSV/software for encoding — independently of each other. FrameMeld auto-detects your GPU(s) and disables encoder options your hardware doesn't support.

## Features

- **Two interpolation engines**: RIFE CUDA (NVIDIA, fastest) and RIFE NCNN (NVIDIA/AMD/Intel via Vulkan), switchable from the main screen
- **Four encoder backends**: NVIDIA NVENC, AMD AMF, Intel QSV, and software (SVT-AV1 / x265 / x264) — each with AV1/HEVC/H.264 options
- GPU auto-detection — encoder options that don't match your installed hardware are automatically disabled, and RIFE CUDA is only selectable when an NVIDIA GPU is actually present
- Hardware-accelerated frame extraction (NVDEC on NVIDIA, DirectX-based decode elsewhere) to reduce CPU load during the extract step
- Vendor-appropriate presets (NVENC's p1–p7, AMF's speed/balanced/quality, QSV/software's veryfast–veryslow) that switch automatically with your encoder choice
- Configurable CQ/CRF
- Auto video info preview (resolution, framerate, frame count, duration) on input selection
- Auto-generated output filename (`{name} {fps}fps.{ext}`) — just pick a folder and format
- Clean, filtered log output (no raw command dumps or FFmpeg banner spam)
- First-launch setup downloads only the runtime components you need (FFmpeg + your chosen interpolation engine) — no separate installs needed
- Temporary working files are cleaned up automatically after a run finishes or is cancelled

## Requirements

- **A GPU with hardware encode support** for your target codec:
  - NVIDIA: NVENC (AV1 NVENC needs RTX 40-series or newer; HEVC/H.264 NVENC go back further) — see [NVIDIA's encoder support matrix](https://developer.nvidia.com/video-encode-and-decode-gpu-support-matrix-new)
  - AMD: AMF-capable GPU for AV1/HEVC/H.264 AMF encoding
  - Intel: QSV-capable GPU for AV1/HEVC/H.264 QSV encoding
  - No hardware encoder? Software encoding (SVT-AV1/x265/x264) works on any CPU, just slower.
- **RIFE CUDA** (optional, NVIDIA only) requires an NVIDIA GPU. **RIFE NCNN** works on NVIDIA/AMD/Intel via Vulkan.
- **Internet connection on first launch**, to download the runtime (see [How the runtime works](#how-the-runtime-works) below).

## 🚀 Download FrameMeld

> ### Ready-to-use Windows build
>
> Download FrameMeld from either of the links below:

### 🟦 GitHub Releases

**[⬇️ Download the latest FrameMeld release](https://github.com/Luxbane/FrameMeld/releases/latest)**

Download the latest Windows build directly from GitHub Releases.

### 🟪 itch.io

**[⬇️ Download FrameMeld from itch.io](https://luxbane.itch.io/framemeld)**

If you prefer downloading the application through itch.io.

## How the runtime works

FrameMeld itself is a small download. The heavier dependencies — FFmpeg, and your chosen RIFE engine (CUDA needs a Python+PyTorch distribution; NCNN is just a small portable executable) — are fetched on first launch and stored in:
```
%LOCALAPPDATA%\FrameMeld\runtime\
```
This keeps the initial download small and lets FrameMeld update those components independently of the app itself. You only need to download the engine you actually plan to use — switching engines later just downloads the other one. The app won't let you start a job until FFmpeg and your selected AI model are downloaded.

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

`dist/`, `build/`, and `.venv/` are generated locally when you build and are not committed — excluded via `.gitignore`. `%LOCALAPPDATA%\FrameMeld\runtime` (the downloaded FFmpeg/RIFE engine files) lives outside the repo entirely, on the end user's machine.

## Usage

1. Launch `FrameMeld.exe`. Pick an interpolation engine from the **Engine** dropdown, then use the **Setup** section to download FFmpeg and that engine's files (only shown when something's missing, or via the **Runtime Downloads** button).
2. Click **Input** and select a video. Its resolution, framerate, frame count, and duration will appear automatically.
3. Click **Output Folder** and pick where the result should be saved.
4. Choose an output **Format** (.mkv / .mp4 / .mov / .webm).
5. Set **Multiplier** (2x/4x), **Scale**, **Encoder**, **Preset**, and **CQ** as needed. Encoder options are automatically limited to what your detected GPU(s) support, plus software encoding.
6. Click **START**. The output filename is generated automatically as `{input name} {new fps}fps.{format}`.

## Roadmap

### 🧠 Interpolation Engines
- [x] RIFE CUDA
- [x] RIFE NCNN/Vulkan

### 🎞️ Video Encoders
- [x] NVIDIA NVENC
- [x] AMD AMF
- [x] Intel QSV
- [x] Software encoding fallback (SVT-AV1 / x265 / x264)

### ⚙️ Runtime & Distribution
- [x] Automatic runtime download
- [x] Per-user LocalAppData runtime
- [x] Hugging Face runtime mirrors
- [x] Per-engine runtime downloads (only fetch what you use)
- [ ] Runtime version management
- [ ] Runtime integrity/hash verification
- [ ] Automatic runtime updates

### 🖥️ Application
- [x] Video metadata detection
- [x] Automatic output naming
- [x] Temporary file cleanup
- [x] GPU auto-detection
- [x] Vendor-aware encoder/preset filtering
- [x] Vendor-aware engine filtering (RIFE CUDA disabled without NVIDIA)
- [x] Hardware-accelerated frame extraction
- [ ] Batch processing
- [ ] Progress estimation
- [ ] Queue system

## Credits & Licenses

FrameMeld is licensed under **GPL-3.0** (see [`LICENSE`](https://github.com/Luxbane/FrameMeld?tab=GPL-3.0-1-ov-file)), as it builds on GPL-3.0-licensed code from Flowframes.

See [`THIRD_PARTY_LICENSES.md`](./THIRD_PARTY_LICENSES.md) for full attribution, including:
- **RIFE** (MIT) — [hzwer/Practical-RIFE](https://github.com/hzwer/Practical-RIFE)
- **RIFE-NCNN-Vulkan** (MIT) — [nihui/rife-ncnn-vulkan](https://github.com/nihui/rife-ncnn-vulkan)
- **Flowframes** (GPL-3.0) — [n00mkrad/flowframes](https://github.com/n00mkrad/flowframes)
- **FFmpeg** (GPL/LGPL) — [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds)
- **7-Zip** (LGPL) — [7-zip.org](https://www.7-zip.org/)

This project is not affiliated with or endorsed by Flowframes, RIFE's authors, FFmpeg, or 7-Zip.