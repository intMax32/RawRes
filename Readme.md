# RawRes

RawRes is a small C++ project for studying the RAW image processing pipeline from Bayer sensor data to a displayable preview image.

The project is intentionally hands-on: instead of relying on a camera JPEG pipeline, it loads RAW data with LibRaw and reconstructs an image step by step with custom processing and OpenCV-based visualization.

## Goals

- understand how RAW sensor data is structured
- inspect Bayer pattern, black level, white level, and as-shot white balance metadata
- build a simple ISP-like preview pipeline
- experiment with white balance and color conversion logic
- use the project as a playground for low-light RAW processing

## Current Status

RawRes is an experimental learning project, not a production RAW converter.

What is implemented today:

- RAW file loading through LibRaw
- extraction of Bayer image data into `cv::Mat`
- Bayer pattern detection
- black level and white level metadata loading
- as-shot white balance gain loading from `cam_mul`
- experimental 3x3 color matrix loading from `cam_xyz`
- Bayer demosaicing using OpenCV
- preview generation for quick inspection
- noise removal routines split into separate modules
- guided filter support for preview denoising

What is still rough or incomplete:

- color reproduction is not yet fully accurate
- color correction / XYZ conversion is still experimental
- BM3D is still a stub
- tone mapping and sharpening are not implemented
- the ISP pipeline is still being refined and debugged

## Dependencies

- C++17
- CMake 3.16+
- OpenCV
- LibRaw

On macOS with Homebrew, the typical setup is:

```bash
brew install opencv libraw
```

On Ubuntu/Debian, a typical setup is:

```bash
sudo apt install cmake g++ libopencv-dev libraw-dev
```

On Windows, install OpenCV and LibRaw first, then point CMake to them if they are
not discoverable automatically.

## Build

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

The executable is built as:

```bash
./build/RawRes
```

On Windows with a multi-config generator such as Visual Studio, the executable is
typically under:

```powershell
build\Debug\RawRes.exe
```

## Run

Pass a RAW file path as the first argument:

```bash
./build/RawRes /path/to/file.ARW
```

You can also run without arguments and enter the path interactively when prompted.

The program currently:

- loads the RAW file
- prints metadata to the terminal
- creates a preview image window with OpenCV

Expected console output is similar to:

```text
Loaded RAW file: /path/to/file.ARW
Size: 4288 x 2848
Black level: 512
White level: 16383
As-shot WB (R/G/B): 1.98 / 1 / 2.19
Bayer pattern : RGGB
```

## Current Pipeline

At a high level, the preview path is:

1. Load RAW data with LibRaw
2. Read Bayer image into `CV_16UC1`
3. Read Bayer pattern and camera metadata
4. Subtract black level and normalize
5. Demosaic Bayer to BGR with OpenCV
6. Apply white balance / experimental color transform
7. Apply gamma correction
8. Display the preview image

## Project Layout

```text
RawRes/
├─ CMakeLists.txt
├─ Readme.md
├─ include/
│  ├─ isp_pipeline.h
│  ├─ raw_loader.h
│  └─ noise_removal/
│     ├─ bm3d.h
│     └─ guided_filter.h
├─ src/
│  ├─ main.cpp
│  ├─ raw_loader.cpp
│  ├─ isp_pipeline.cpp
│  └─ noise_removal/
│     ├─ bm3d.cpp
│     └─ guided_filter.cpp
├─ data/
├─ results/
└─ build/
```

## Important Files

- `src/raw_loader.cpp`
  Loads RAW files through LibRaw and extracts sensor data plus metadata.

- `src/isp_pipeline.cpp`
  Contains the preview pipeline: normalization, demosaicing, white balance, color conversion, gamma correction, and optional denoising dispatch.

- `src/noise_removal/guided_filter.cpp`
  Guided filter implementation used by the preview pipeline when denoising is enabled.

- `src/noise_removal/bm3d.cpp`
  Placeholder for BM3D. The function exists, but the algorithm is not implemented yet.

- `src/main.cpp`
  Entry point for loading a RAW file and showing a preview window.

## Notes

- The `data/` directory is intended for local RAW datasets and should not be committed to GitHub.
- `cam_mul` is used as the source of as-shot white balance gains.
- `cam_xyz` is currently loaded into a 3x3 matrix for experimentation, but the color pipeline is still under investigation.

## Roadmap

Likely next steps for the project:

- make the preview path more stable and color-accurate
- separate camera RGB, XYZ, and display RGB conversions more clearly
- add optional intermediate-stage visualization
- add white balance sliders or interactive controls
- implement BM3D
- add tone mapping and sharpening stages
- compare the custom ISP output against camera JPEGs or LibRaw processed output

## License

No license file has been added yet.
