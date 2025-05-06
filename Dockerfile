# ================================
# Build image
# ================================
FROM alpine:latest AS build

RUN apk add --no-cache curl
RUN echo "https://portsman.anamy.gay" | tee -a /etc/apk/repositories 
RUN mkdir -p /etc/apk/keys && curl -o /etc/apk/keys/portsman-anamy.rsa.pub https://raw.githubusercontent.com/elihwyma/portsman/refs/heads/main/portsman-anamy.rsa.pub

RUN apk update && apk add --no-cache \
    cmake \
    git \
    clang17 \
    pkgconf \
    opencv-dev \ 
    libstdc++ \
    samurai \
    i2c-tools-dev \
    libgpiod-dev=1.6.5-r0 \
    libdxl-dev \
    tensorflowlite-dev \
    flatbuffers-dev

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
RUN cp -r /build/resources resources

# ================================
# Final image
# ================================
FROM alpine:latest AS build

RUN apk update && apk add --no-cache \
    libopencv_core \
    libstdc++ \
    i2c-tools \
    libgpiod=1.6.5-r0 \
    libdxl \
    tensorflowlite \
    flatbuffers

# Set up the runtime environment
WORKDIR /app

# Copy the runtime files
COPY --from=build /staging/* .

# Run the application
ENTRYPOINT ["./runtime"]