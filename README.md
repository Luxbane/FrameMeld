---
license: mit
---
# FrameMeld

RIFE video frame interpolation with hardware-accelerated encoding (AV1 / HEVC / H.264), wrapped in a simple desktop GUI.

FrameMeld integrates RIFE AI frame interpolation (CUDA & NCNN/Vulkan) with GPU hardware-accelerated video encoding (AV1 / HEVC / H.264 via FFmpeg) in a lightweight, native desktop GUI — no manual frame extraction, no terminal juggling.

> **NVIDIA, AMD, and Intel GPUs are supported.** Pick RIFE CUDA (NVIDIA only, fastest) or RIFE NCNN (NVIDIA/AMD/Intel via Vulkan) for interpolation, and NVENC/AMF/QSV/software for encoding — independently of each other. FrameMeld auto-detects your GPU(s) and disables encoder options your hardware doesn't support.

---

## ⚠️ CRITICAL THERMAL WARNING — Read Before Running

> [!CAUTION]
> ### 🛑 STRONGLY DISADVISED: Thin Laptops with Integrated GPU (iGPU Only)
> **Running FrameMeld on thin-and-light laptops or ultrabooks that only rely on an integrated GPU (Intel Arc Graphics, Intel Iris Xe, AMD Radeon 600M/700M/800M iGPU) without dedicated cooling is STRONGLY DISADVISED.**
>
> AI frame interpolation puts an extreme, sustained neural network compute workload on GPU execution units. In compact laptops, the CPU and iGPU share a single thermal block and small heatpipes. During real-world testing, running RIFE NCNN on a thin laptop's integrated GPU caused iGPU die temperatures to spike to an alarming **107°C**! Sustained operation at such temperatures can lead to emergency shutdowns, severe thermal throttling, and permanent hardware degradation.
>
> **FrameMeld is intended for Desktop PCs and Gaming Laptops equipped with dedicated GPUs (dGPU) and robust cooling systems.**

> ❄️ **CPU about to melt into molten lava?**  
> Check out **[FrameMeld Pro on itch.io](https://luxbane.itch.io/framemeld)** ($5).  
> The Pro version features the exclusive **RIFE CUDA Optimized Direct Streaming Pipe** — streaming frames directly through RAM memory without writing tens of thousands of temporary PNG files to SSD. It dramatically reduces CPU load, keeping temperatures cool at **60–75°C** (instead of 100°C) while letting your NVIDIA GPU run at full speed!

### Before running long jobs on any laptop:
- **Set a strict power or temperature limit**: If your laptop has vendor software (e.g. Lenovo Vantage / Legion Toolkit, ASUS Armoury Crate / G-Helper, MSI Center), set a temperature cap or wattage limit (PL1/TDP) before starting.
- **MSI Afterburner**: Lower the Power Limit slider to cap GPU power draw.
- **Windows Power Options**: Lower *Maximum processor state* to 80–85% to disable aggressive CPU boost clocking.
- Make sure laptop intake and exhaust vents are elevated and unblocked; consider using an active cooling pad.

**We are not responsible for thermal damage, reduced hardware lifespan, or any consequences of running FrameMeld on thermally constrained or uncooled hardware. Use at your own risk.**

---

## Features

- **Two interpolation engines**: RIFE CUDA (NVIDIA, fastest) and RIFE NCNN (NVIDIA/AMD/Intel via Vulkan), switchable from the main screen
- **Four encoder backends**: NVIDIA NVENC, AMD AMF, Intel QSV, and software (SVT-AV1 / x265 / x264) — each with AV1/HEVC/H.264 options
- **GPU auto-detection** — encoder options that don't match your installed hardware are automatically disabled, and RIFE CUDA is only selectable when an NVIDIA GPU is actually present
- **Hardware-accelerated frame extraction** (NVDEC on NVIDIA, DirectX-based decode elsewhere) to reduce CPU load during the extract step
- **Vendor-appropriate presets** (NVENC's p1–p7, AMF's speed/balanced/quality, QSV/software's veryfast–veryslow) that switch automatically with your encoder choice
- **Configurable CQ/CRF**
- **Auto video info preview** (resolution, framerate, frame count, duration) on input selection
- **Auto-generated output filename** (`{name} {fps}fps.{ext}`) — just pick a folder and format
- **Native C++ Performance**: Instant startup (<0.05s) and minimal RAM footprint (~25 MB) built with native Win32 & C++17
- **Light & Dark Mode**: One-click theme switcher with Windows system theme auto-detection and settings persistence
- **Dynamic Auto-Collapsing Downloads Panel**: Seamlessly hides when all runtime tools are ready, expanding the log view area
- **Clean, filtered log output** with real-time percentage progress tracking
- **First-launch setup** downloads only the runtime components you need (FFmpeg + your chosen interpolation engine) — no separate installs needed
- **Temporary working files** are cleaned up automatically after a run finishes or is cancelled

---

## Requirements

- **A GPU with hardware encode support** for your target codec:
  - NVIDIA: NVENC (AV1 NVENC needs RTX 40-series or newer; HEVC/H.264 NVENC go back further) — see [NVIDIA's encoder support matrix](https://developer.nvidia.com/video-encode-and-decode-gpu-support-matrix-new)
  - AMD: AMF-capable GPU for AV1/HEVC/H.264 AMF encoding
  - Intel: QSV-capable GPU for AV1/HEVC/H.264 QSV encoding
  - No hardware encoder? Software encoding (SVT-AV1/x265/x264) works on any CPU, just slower.
- **RIFE CUDA** (optional, NVIDIA only) requires an NVIDIA GPU. **RIFE NCNN** works on NVIDIA/AMD/Intel via Vulkan.
- **Internet connection on first launch**, to download the runtime (see [How the runtime works](#how-the-runtime-works) below).

---

## 🚀 Download FrameMeld

> ### Ready-to-use Windows build
> Download FrameMeld from either of the links below:

### 🟦 GitHub Releases
**[⬇️ Download the latest FrameMeld free version release](https://github.com/Luxbane/FrameMeld/releases/latest)**

### 🟪 itch.io
**[⬇️ Download FrameMeld from itch.io (Download the demo version for free)](https://luxbane.itch.io/framemeld)**

---

## How the Runtime Works

FrameMeld itself is a small download. The heavier dependencies — FFmpeg, and your chosen RIFE engine (CUDA needs a Python+PyTorch distribution; NCNN is just a small portable executable) — are fetched on first launch and stored in:
```
%LOCALAPPDATA%\FrameMeld\runtime\
```
This keeps the initial download small and lets FrameMeld update those components independently of the app itself. You only need to download the engine you actually plan to use — switching engines later just downloads the other one. The app won't let you start a job until FFmpeg and your selected AI model are downloaded.

These files are mirrored (as `.7z` archives) from their original sources on [Hugging Face](https://huggingface.co/Luxbane/FrameMeld/tree/main) — see [Credits & Licenses](#credits--licenses) for where each component actually comes from.

---

## Building from Source

Only needed if you want to modify FrameMeld, verify the build, or produce your own release. If you just want to use the app, use the [Download](#-download-framemeld) section above instead.

Requirements for building:
1. **MinGW-w64 `g++`** (supporting C++17) available on your system PATH.

Steps:
1. Clone this repo.
2. Run `Build_FrameMeld.bat`.

The script will:
- Compile `src/main.cpp` using `g++` (C++17, `-O3`, `-mwindows`, standalone with no runtime DLL dependencies)
- Output `dist/FrameMeld/FrameMeld.exe` and prepare `dist/FrameMeld/tools/7za.exe`

---

## Usage

1. Launch `FrameMeld.exe`. Pick an interpolation engine from the **Engine** dropdown, then use the **Setup** section to download FFmpeg and that engine's files (only shown when something's missing, or via the **Runtime Downloads** button).
2. Click **Input** and select a video. Its resolution, framerate, frame count, and duration will appear automatically.
3. Click **Output Folder** and pick where the result should be saved.
4. Choose an output **Format** (.mkv / .mp4 / .mov / .webm).
5. Set **Multiplier** (2x/4x), **Scale**, **Encoder**, **Preset**, and **CQ** as needed. Encoder options are automatically limited to what your detected GPU(s) support, plus software encoding.
6. Click **START**. The output filename is generated automatically as `{input name} {new fps}fps.{format}`.

---

## Credits & Licenses

FrameMeld is open-source software licensed under the **MIT License** (see [`LICENSE`](./LICENSE)).

- GUI FrameMeld C++: Luxbane (MIT)
- **RIFE CUDA**: [Practical-RIFE](https://github.com/hzwer/Practical-RIFE) by Zhewei Huang (MIT)
- **RIFE Model & Core Architecture**: [hzwer/Practical-RIFE](https://github.com/hzwer/Practical-RIFE) by Zhewei Huang (MIT)
- **RIFE NCNN Vulkan**: [nihui/rife-ncnn-vulkan](https://github.com/nihui/rife-ncnn-vulkan) by nihui (MIT)
- **FFmpeg**: [FFmpeg Developers](https://ffmpeg.org/) / [BtbN Builds](https://github.com/BtbN/FFmpeg-Builds) (GPL-3.0)
- **7-Zip / 7za.exe**: [Igor Pavlov](https://www.7-zip.org/) (LGPL-2.1-or-later)

See [`THIRD_PARTY_LICENSES.md`](./THIRD_PARTY_LICENSES.md) for full third-party notices and license texts.

## Acknowledgements
- Special thanks to [Flowframes](https://github.com/n00mkrad/flowframes) by n00mkrad for inspiration.

This project is not affiliated with or endorsed by Practical-RIFE, Tencent NCNN, FFmpeg, or 7-Zip.
