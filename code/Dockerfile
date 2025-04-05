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

# Pre-Download dynamixel
RUN git clone --depth=1 --branch "3.8.2" "https://github.com/ROBOTIS-GIT/DynamixelSDK.git" dynamixel_src

# Copy the source code
COPY . .

# Generate the project
RUN cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release

# Build the project
RUN cmake --build build

# Switch to the staging area
WORKDIR /staging

RUN cp /build/build/runtime .
RUN cp -r /build/resources .

# ================================
# Final image
# ================================
FROM ghcr.io/dtcooper/raspberrypi-os:latest

RUN apt-get update && apt-get upgrade -y && apt-get install -y \
    libopencv-dev \
    wget \
    libgpiod-dev

# Install TensorFlow Lite
RUN wget https://github.com/prepkg/tensorflow-lite-raspberrypi/releases/latest/download/tensorflow-lite_64.deb && apt-get install -y ./tensorflow-lite_64.deb

# Set up the runtime environment
WORKDIR /app

# Copy the runtime files
COPY --from=build /staging/* .

# Run the application
ENTRYPOINT ["./runtime"]