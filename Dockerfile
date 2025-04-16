# ================================
# Build image
# ================================
FROM ghcr.io/dtcooper/raspberrypi-os:latest AS build

# Install OS updates
RUN apt-get update && apt-get upgrade -y --no-install-recommends --no-install-suggests && apt-get install -y --no-install-recommends --no-install-suggests \
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
    libi2c0 \
    libgpiod-dev && apt-get clean autoclean && apt-get autoremove --yes && rm -rf /var/lib/{apt,dpkg,cache,log}/

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
RUN cp -r /build/resources resources

# ================================
# Final image
# ================================
FROM ghcr.io/dtcooper/raspberrypi-os:python3.11-bookworm

RUN apt-get update && apt-get upgrade -y --no-install-recommends --no-install-suggests && apt-get install -y --no-install-recommends --no-install-suggests \
    libopencv-core406 \
    libopencv-videoio406 \
    wget \
    libgpiod2 \
    python3-numpy \
    libi2c0 \
    ca-certificates && apt-get clean autoclean && apt-get autoremove --yes && rm -rf /var/lib/{apt,dpkg,cache,log}/

# Install TensorFlow Lite
RUN wget https://github.com/prepkg/tensorflow-lite-raspberrypi/releases/latest/download/tensorflow-lite_64.deb && dpkg -i tensorflow-lite_64.deb && rm tensorflow-lite_64.deb

# Set up the runtime environment
WORKDIR /app

# Copy the runtime files
COPY --from=build /staging/* .

# Run the application
ENTRYPOINT ["./runtime"]