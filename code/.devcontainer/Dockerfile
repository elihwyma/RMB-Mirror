# ================================
# Build image
# ================================
FROM ghcr.io/dtcooper/raspberrypi-os:latest AS build

# Install OS updates
RUN apt-get update && apt-get upgrade -y && apt-get install -y \
    cmake \
    make \
    git \
    python3 \
    python3-pip \
    build-essential \
    clang \
    pkg-config \
    libopencv-dev \
    ninja-build \
    wget \
    libgpiod-dev

# Set up a build area
WORKDIR /build

# Install TensorFlow Lite
RUN wget https://github.com/prepkg/tensorflow-lite-raspberrypi/releases/latest/download/tensorflow-lite_64.deb && apt-get install -y ./tensorflow-lite_64.deb
