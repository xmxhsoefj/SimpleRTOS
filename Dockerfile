FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    gcc-arm-none-eabi \
    gdb-arm-none-eabi \
    libnewlib-arm-none-eabi \
    openocd \
    make \
    cmake \
    git \
    nano \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /project
