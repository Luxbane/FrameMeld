# Third-Party Licenses & Attributions

FrameMeld is open-source software licensed under the **MIT License** (see [`LICENSE`](./LICENSE)).

This document details the copyright notices, upstream sources, and licenses for the third-party components, runtimes, models, and tools used by or distributed alongside FrameMeld.

---

## 1. FrameMeld Native GUI
- **Author**: Luxbane
- **License**: MIT License
- **Description**: Standalone C++17 Win32 graphical user interface, process pipeline manager, and hardware capability detector.

---

## 2. RIFE CUDA Wrapper (`rife.py`)
- **Upstream Project**: [Practical-RIFE](https://github.com/hzwer/Practical-RIFE)
- **Author**: Zhewei Huang (hzwer) / Luxbane
- **License**: MIT License
- **Description**: Python CLI wrapper and interpolation pipeline driving RIFE AI models on NVIDIA CUDA hardware. Built upon Practical-RIFE's inference architecture with custom Windows path encoding, multithreaded frame generation queues, and real-time IPC progress reporting.

---

## 3. RIFE Neural Network Architecture & Model Weights
- **Upstream Project**: [Practical-RIFE](https://github.com/hzwer/Practical-RIFE)
- **Author**: Zhewei Huang (hzwer)
- **License**: MIT License
- **Description**: Real-Time Intermediate Flow Estimation for Video Frame Interpolation (RIFE) neural network architecture and pre-trained model checkpoints.

```
MIT License

Copyright (c) 2020 hzwer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 4. RIFE NCNN Vulkan
- **Upstream Project**: [rife-ncnn-vulkan](https://github.com/nihui/rife-ncnn-vulkan)
- **Author**: nihui
- **License**: MIT License
- **Description**: Portable RIFE frame interpolation implementation optimized for Tencent's NCNN neural network computing framework with Vulkan GPU acceleration (supporting NVIDIA, AMD, and Intel GPUs).

```
MIT License

Copyright (c) 2020 nihui

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 5. FFmpeg
- **Upstream Project**: [FFmpeg](https://ffmpeg.org/)
- **Binary Provider**: [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds)
- **License**: GNU General Public License v3.0 (GPL-3.0)
- **Description**: Multimedia framework used for video decoding, frame extraction, hardware acceleration probing, and final video encoding (NVENC, AMF, QSV, SVT-AV1, x265, x264).
- **Build Flags**: The downloaded Windows release is built with `--enable-gpl --enable-version3`. Source code for FFmpeg and build scripts are available at the upstream repository.

---

## 6. 7-Zip (`7za.exe`)
- **Upstream Project**: [7-Zip](https://www.7-zip.org/)
- **Author**: Igor Pavlov
- **License**: GNU LGPL-2.1-or-later (with BSD 3-Clause and unRAR license exceptions)
- **Description**: Standalone console archive utility (`7za.exe`) bundled to unpack downloaded runtime `.7z` packages upon first launch.

---

## 7. Hugging Face Runtime Mirrors
- **Repository**: [Luxbane/FrameMeld on Hugging Face](https://huggingface.co/Luxbane/FrameMeld)
- **Description**: Pre-packaged `.7z` runtime archives for FFmpeg, the portable Python/PyTorch CUDA environment, and RIFE NCNN Vulkan binaries are hosted for user download convenience. These archives contain unmodified third-party binaries and models under their respective licenses listed above. No proprietary claim is made over mirrored components.

---

## Acknowledgements
- [Flowframes](https://github.com/n00mkrad/flowframes) by n00mkrad for workflow and UI inspiration.

---

## Disclaimer
FrameMeld is an independent project and is not affiliated with, endorsed by, or sponsored by Practical-RIFE, Tencent NCNN, FFmpeg, 7-Zip, or Flowframes.
