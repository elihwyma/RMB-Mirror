# ================================
# Build image
# ================================
FROM alpine:latest AS build

RUN apk add --no-cache curl && mkdir -p /etc/apk/keys && curl -o /etc/apk/keys/portsman-anamy.rsa.pub https://raw.githubusercontent.com/elihwyma/portsman/refs/heads/main/portsman-anamy.rsa.pub
RUN echo "https://portsman.anamy.gay" | tee -a /etc/apk/repositories 

RUN apk update && apk add --no-cache \
    cmake \
    git \
    clang19 \
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
RUN CXX=clang-19 cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug

# Build the project
RUN cmake --build build

# Switch to the staging area
WORKDIR /staging

RUN cp /build/build/runtime .
RUN cp -r /build/resources resources

# ================================
# Final image
# ================================
FROM alpine:latest

RUN apk add --no-cache curl && mkdir -p /etc/apk/keys && curl -o /etc/apk/keys/portsman-anamy.rsa.pub https://raw.githubusercontent.com/elihwyma/portsman/refs/heads/main/portsman-anamy.rsa.pub
RUN echo "https://portsman.anamy.gay" | tee -a /etc/apk/repositories 

RUN apk update && apk add --no-cache \
    libopencv_videoio \
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