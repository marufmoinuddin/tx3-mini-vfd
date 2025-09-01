# TX3 Mini VFD Controller (C++ Version)

A lightweight C++ implementation of the VFD (Vacuum Fluorescent Display) controller for TX3 Mini devices with 4-digit 7-segment displays.

## Features

- **Clock Display**: 12-hour format with blinking colon (10 seconds)
- **System Stats**: CPU temperature, memory usage, storage usage (4+3+3 seconds)
- **Status LEDs**: Network status, temperature warnings, disk/memory alerts
- **Lightweight**: Minimal resource usage compared to Python version
- **Signal Handling**: Graceful shutdown with Ctrl+C

## Requirements

- **Hardware**: TX3 Mini with VFD display
- **OS**: Linux (tested on Armbian)
- **Compiler**: g++ with C++17 support
- **Permissions**: Root access required for hardware control

## Compilation Instructions

### Method 1: Using Makefile (Recommended)

```bash
# Navigate to the project directory
cd /mnt/Storage/Studies/randoms/vfd_cpp

# Compile the program
make

# Install system-wide (optional)
make install

# Create systemd service (optional)
make service
```

### Method 2: Manual Compilation

```bash
# Basic compilation
g++ -std=c++17 -O2 -Wall -Wextra -pthread -o vfd_controller vfd_controller.cpp

# With debug symbols (for development)
g++ -std=c++17 -g -Wall -Wextra -pthread -o vfd_controller vfd_controller.cpp

# Static linking (for portability)
g++ -std=c++17 -O2 -static -pthread -o vfd_controller vfd_controller.cpp
```

### Method 3: Cross-compilation (if building on different architecture)

```bash
# For ARM targets (adjust toolchain as needed)
arm-linux-gnueabihf-g++ -std=c++17 -O2 -static -pthread -o vfd_controller vfd_controller.cpp
```

## Usage

### Running the Program

```bash
# Direct execution (requires root)
sudo ./vfd_controller

# If installed system-wide
sudo vfd_controller
```

### Running as System Service

#### Quick Installation (Recommended)
```bash
# Use the automated installer
./install_service.sh
```

#### Manual Installation
```bash
# Compile and install
make install

# Install and enable systemd service
make enable-service

# Or install service without auto-enabling
make service
sudo systemctl enable vfd-controller.service
sudo systemctl start vfd-controller.service
```

#### Service Management Commands
```bash
# Check service status
make status
# OR
sudo systemctl status vfd-controller.service

# View real-time logs
make logs
# OR
sudo journalctl -u vfd-controller.service -f

# Stop the service
make stop
# OR
sudo systemctl stop vfd-controller.service

# Restart the service
make restart
# OR
sudo systemctl restart vfd-controller.service

# Disable service (stop auto-start)
sudo systemctl disable vfd-controller.service

# Complete uninstall
make uninstall
```

### Stopping the Program

- **Interactive**: Press `Ctrl+C`
- **Service**: `sudo systemctl stop vfd-controller.service`
- **Kill Process**: `sudo pkill vfd_controller`

## Display Timing

The program cycles through different displays:

1. **Clock**: 10 seconds (shows time in 12-hour format)
2. **CPU Temperature**: 4 seconds (shows "C XX")
3. **Memory Usage**: 3 seconds (shows "r XX")
4. **Storage Usage**: 3 seconds (shows "S XX")

**Total cycle**: 20 seconds

## LED Indicators

- **colon**: Blinks with clock, solid during stats
- **play**: AM/PM indicator during clock
- **alarm**: Temperature warning (>70°C)
- **usb**: High disk usage (>80%)
- **wlan**: WiFi connection status
- **lan**: Ethernet connection status
- **pause**: High memory usage (>80%)

## Building Optimizations

### Size Optimization
```bash
g++ -std=c++17 -Os -s -DNDEBUG -pthread -o vfd_controller vfd_controller.cpp
strip vfd_controller
```

### Performance Optimization
```bash
g++ -std=c++17 -O3 -march=native -flto -pthread -o vfd_controller vfd_controller.cpp
```

## Troubleshooting

### Common Issues

1. **Permission Denied**: Run with `sudo`
2. **Display Not Found**: Check if `/sys/devices/platform/spi/spi_master/spi0/spi0.0/display_text` exists
3. **LEDs Not Working**: Verify LED paths in `/sys/class/leds/`
4. **Temperature Not Reading**: Install `lm-sensors` or check hardware sensors

### Debugging

```bash
# Compile with debug symbols
g++ -std=c++17 -g -DDEBUG -pthread -o vfd_controller vfd_controller.cpp

# Run with strace to see system calls
sudo strace -e trace=openat,write ./vfd_controller

# Check system paths
ls -la /sys/devices/platform/spi/
ls -la /sys/class/leds/
```

## File Structure

```
vfd_cpp/
├── vfd_controller.cpp    # Main source code
├── Makefile             # Build configuration
└── README.md           # This file
```

## Comparison with Python Version

| Feature | C++ Version | Python Version |
|---------|-------------|----------------|
| Memory Usage | ~2-5 MB | ~15-25 MB |
| CPU Usage | ~0.1-0.5% | ~1-3% |
| Startup Time | <100ms | ~1-2s |
| Dependencies | None (static) | Python3 + modules |
| Binary Size | ~100-500KB | N/A |

## License

This code is provided as-is for educational and personal use.
