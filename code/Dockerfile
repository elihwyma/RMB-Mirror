# ================================
# Build image
# ================================
FROM ubuntu:latest AS build

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
    ninja-build

# Set up a build area
WORKDIR /build

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
FROM ubuntu:latest

RUN apt-get update && apt-get upgrade -y && apt-get install -y \
    libopencv-dev

# Set up the runtime environment
WORKDIR /app

# Copy the runtime files
COPY --from=build /staging/* .

# Run the application
ENTRYPOINT ["./runtime"]