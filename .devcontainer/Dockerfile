# The standard CS 1410 image, plus what a raylib game needs to build and draw:
# X11 + OpenGL headers, and Mesa's software renderer (llvmpipe) so the game
# runs with no GPU at all -- containers don't get one.
# mesa-utils provides glxinfo, which the smoke test reports.
FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

RUN apt-get update \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
        libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
        libgl1-mesa-dev libgl1-mesa-dri \
        mesa-utils \
    && rm -rf /var/lib/apt/lists/*
