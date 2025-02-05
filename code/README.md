# COMP209

## Code

### Building Docker Container

```bash 
docker buildx build --platform linux/arm64 . -t docker.github.falmouth.ac.uk/aw295676/comp209-team/runtime:dev && docker run docker.github.falmouth.ac.uk/aw295676/comp209-team/runtime:dev
```