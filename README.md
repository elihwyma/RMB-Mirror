# RateMyBot

## Program

The program for RateMyBot is a C++ program using [Google's MediaPipe](https://github.com/google-ai-edge/mediapipe) to generate meshes of presented faces.

To simplify development the program can be built inside a Docker container. This is highly recommended on Windows based systems.

On Windows it is recommened to use [Docker Dekstop](https://docs.docker.com/desktop/setup/install/windows-install/).
On macOS it is recommened to use [OrbStack](https://orbstack.dev).

### Docker

1: Clone the repository
  ```
  git clone https://github.falmouth.ac.uk/AW295676/COMP209-team
  ```
2: Navigate into the code directory
   ```
   cd COMP209-team/code
   ```
3: Build the docker container
   ```
   docker build . -t runtime-dev
   ```
4: Run the ducker container
   ```
   docker run runtime-dev
   ``` 

### Bare-metal build

**macOS**:
1. Install the latest Xcode version. For this it is recommended to use [XcodesApp](https://github.com/XcodesOrg/XcodesApp/releases).
2. Install required dependencies
   ```
   brew install pkg-config cmake make git ninja
   ```

**Debian/Ubuntu**:
1. Install required dependencies
   ```
   sudo apt update && sudo apt install -y cmake make python3 git python3-pip build-essential clang pkg-config ninja
   ```

**Arch**:
1. Install required dependencies
   ```
   sudo pacman -Sy cmake make python3 git python3-pip base-devel clang pkg-config ninja
   ```

**Building**
1. Generate build files
   ```
   cmake -G Ninja -B build
   ```
2. Build product
   ```
   cmake --build build
   ```
3. Run the build
   ```
   build/runtime
   ```

## License

This project is licensed under the Apache 2.0 License. It's full terms can be read in [LICENSE](LICENSE). This project uses the following open source projects:

| Project | License | How it's used |
| - | - | - |
| [Tensorflow](https://github.com/tensorflow/tensorflow) | [Apache 2.0](https://github.com/tensorflow/tensorflow/blob/master/LICENSE) | Running inference on TensorflowLite model |
| [MediaPipe](https://github.com/google-ai-edge/mediapipe) | [Apache 2.0](https://github.com/google-ai-edge/mediapipe/blob/master/LICENSE) | The Face Mesh model for identifiying the landmarks of the user |
