# ================================
# Build image
# ================================
FROM alpine:latest AS build

# Install OS updates
RUN apk update && apk upgrade && apk add --no-cache \
    cmake \
    g++ \
    make \
    pkgconfig \
    libffi \ 
    git

# Set up a build area
WORKDIR /build

# Clone in Tensorflow Lite
RUN git clone --branch v2.18.0 --depth 1 https://github.com/tensorflow/tensorflow.git tensorflow_src

# Copy the source code
COPY . .

# Build the project
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build

# Switch to the staging area
WORKDIR /staging

RUN cp /build/build/runtime .

# ================================
# Final image
# ================================
FROM alpine:latest

# Set up the runtime environment
WORKDIR /app

# Copy the runtime files
COPY --from=build /staging/runtime .

# Run the application
ENTRYPOINT ["./runtime"]