# Third-Party Licenses

FrameMeld is licensed under GPL-3.0 (see [`LICENSE`](./LICENSE)). It builds on and redistributes components from the following third-party projects.

## RIFE

Copyright (c) 2020 hzwer
Licensed under the **MIT License**.
Source: https://github.com/hzwer/Practical-RIFE

The RIFE model and CUDA implementation used by FrameMeld are RIFE's, integrated via Flowframes' packaging of it (see below).

## Flowframes

Copyright (c) n00mkrad
Licensed under **GPL-3.0**.
Source: https://github.com/n00mkrad/flowframes

FrameMeld uses a modified copy of Flowframes' `rife.py` CLI wrapper and its bundled Python (`py-amp`) runtime, which packages RIFE with the PyTorch/CUDA dependencies needed to run it. Because this code is GPL-3.0, FrameMeld is licensed under GPL-3.0 as well, and its full source is available in this repository.

## FFmpeg

Licensed under **GPL** (build configuration dependent; the specific build FrameMeld downloads is compiled with `--enable-gpl --enable-version3`).
BtbN builds used: https://github.com/BtbN/FFmpeg-Builds

FFmpeg is downloaded by FrameMeld at first launch and used for frame extraction and final video encoding (including NVENC).

## 7-Zip / 7za.exe

Licensed under **LGPL** (with some components under BSD-3-Clause/unRAR license restrictions that don't apply to the plain LGPL parts FrameMeld uses).
Source: https://www.7-zip.org/

The standalone `7za.exe` console tool is bundled with FrameMeld to extract the downloaded runtime archives (`.7z` files).

## Runtime mirror on Hugging Face

The FFmpeg, Python runtime (`py-amp`), and RIFE CUDA files that FrameMeld downloads on first launch are re-hosted (as `.7z` archives, unmodified except for compression) at:
https://huggingface.co/Luxbane/FrameMeld-runtime

This mirror exists purely for distribution convenience — the underlying components and their licenses are exactly as described above. It is not an independent claim of authorship over any of these components.

---

This project is not affiliated with or endorsed by Flowframes, RIFE's authors, FFmpeg, or 7-Zip. If you are a rights holder and have concerns about how your project is credited or redistributed here, please open an issue.