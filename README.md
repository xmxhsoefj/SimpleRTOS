# SimpleRTOS
Custom RTOS Implementation for STM32 F769I-DISCO for the sake of implementing RTOS

## Setup
1. Check `lsusb` & `udevadm info --query=all --name=/dev/bus/usb/0../0..`
2. In `/etc/udev/rules.d/99-stlink.rules`:
```
SUBSYSTEM=="usb", ATTR{idVendor}=="0483", ATTR{idProduct}=="374b", MODE="0666"
```
3. Reload pc or exec:
```
sudo udevadm control --reload-rules && sudo udevadm trigger
```
And replug the board
4.
```
sudo usermod -aG plugdev $USER
```

## Build
1. Build and enter the container:
```
podman build -t simple-rtos .
podman run --rm -it --privileged \
  --device=/dev/bus/usb/003/006 \
  -v "$(pwd)":/project \
  simple-rtos bash
```
2. Compile and flash the board via:
```
cmake -B build
# cmake --build build
cmake --build build --target flash
```

## Development
If container is used for building the project, and you want lsp to work,
compile_commands shall be fixed with:
```
ln -s build/compile_commands.json .
sed -i 's|/project|'$(pwd)'|g' build/compile_commands.json
```
