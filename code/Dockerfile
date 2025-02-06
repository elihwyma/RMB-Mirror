# ================================
# Build image
# ================================
FROM ubuntu:latest AS build

# Install OS updates
RUN apt update && apt upgrade -y && apt install -y \
    cmake \
    make \
    git \
    python3 \
    python3-pip \
    build-essential \
    clang \
    pkg-config

# Set up a build area
WORKDIR /build

# Copy the source code
COPY . .

# Generate the project
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build the project
RUN cmake --build build

# Switch to the staging area
WORKDIR /staging

RUN cp /build/build/runtime .
RUN cp /build/resources/* .

# ================================
# Final image
# ================================
FROM ubuntu:latest

# Set up the runtime environment
WORKDIR /app

# Copy the runtime files
COPY --from=build /staging/* .

# Run the application
ENTRYPOINT ["./runtime"]