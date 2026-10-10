# libdrm dumb framebuffer demonstration

## Function
- Program automatically selects an active drm_connector+CRTC+mode combination (highest res).
- Briefly displays a red box in the top left corner for 10 seconds
- Exits

## Build
```bash
# compile
make

# clean
make clean

# run
./main
```

make sure no other window systems or desktop environments are running!
