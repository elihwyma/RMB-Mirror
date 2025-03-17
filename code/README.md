# COMP209

## Code

### Building Docker Container

```bash 
docker buildx build --platform linux/arm64 . -t docker.github.falmouth.ac.uk/aw295676/comp209-team/runtime:dev && docker run docker.github.falmouth.ac.uk/aw295676/comp209-team/runtime:dev
```

## Dynamixel

References: 
https://emanual.robotis.com/docs/en/dxl/ax/ax-12a/#control-table-description
https://github.com/ROBOTIS-GIT/DynamixelSDK/blob/main/c%2B%2B/example/protocol1.0/read_write/read_write.cpp