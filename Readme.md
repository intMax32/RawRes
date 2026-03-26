# RawRes

RawRes is a C++ project for exploring the RAW image processing pipeline from sensor data to displayable images.

The goal of this project is to build a lightweight **RAW reader + ISP simulator** focused on understanding and experimenting with the core stages of image formation, especially in **low-light RAW imaging**.

Rather than treating RAW files as already-finished images, RawRes works directly with sensor-like data and reconstructs a preview image step by step through a manually implemented pipeline.

---

## Motivation

Modern cameras and smartphones do not produce final images directly from the sensor.  
Instead, they generate RAW data that must go through an **ISP (Image Signal Processing) pipeline**, including steps such as:

- black level subtraction
- normalization
- white balance
- demosaicing
- color correction
- tone mapping
- gamma correction

This project exists to better understand that pipeline by implementing it manually and making each stage observable and debuggable.

In the long term, RawRes is also intended to become a small research-oriented playground for:

- low-light RAW visualization
- ISP simulation
- RAW color pipeline analysis
- low-light RAW restoration experiments

---

## Current Features

At the current stage, RawRes supports:

- loading RAW image files through **LibRaw**
- reading Bayer RAW data and metadata
- black level subtraction
- normalization using black/white levels
- Bayer demosaicing with OpenCV
- manual white balance gain
- experimental color correction / camera RGB to XYZ conversion
- gamma correction
- preview generation and image export

---

## Planned Features

The following features are planned for future versions:

- automatic Bayer pattern detection from RAW metadata
- automatic white balance
- better color correction pipeline
- proper tone curve / tone mapping
- denoising stage
- sharpening stage
- support for low-light RAW restoration experiments
- stage-by-stage visualization of the ISP pipeline
- comparison mode for intermediate pipeline outputs

---

## Pipeline Overview

The current pipeline is roughly:

1. Load RAW file
2. Extract Bayer image and metadata
3. Subtract black level
4. Normalize by white level
5. Demosaic Bayer image
6. Apply white balance
7. Apply color conversion / correction
8. Apply gamma correction
9. Export preview image

In the future, the pipeline will be extended toward a more complete ISP simulation.

---

## Tech Stack

- **C++17**
- **CMake**
- **OpenCV**
- **LibRaw**

---

## Project Structure

```text
RawRes/
├─ CMakeLists.txt
├─ include/
│  ├─ raw_loader.h
│  └─ isp_pipeline.h
├─ src/
│  ├─ main.cpp
│  ├─ raw_loader.cpp
│  └─ isp_pipeline.cpp
├─ data/
├─ outputs/
└─ build/