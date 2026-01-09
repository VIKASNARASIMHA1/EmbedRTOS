# EmbedRTOS
EmbedRTOS is a comprehensive embedded systems simulator designed as a portfolio project to showcase professional embedded software engineering skills. It implements a complete embedded ecosystem including a custom RTOS, hardware abstraction layer, sensor fusion algorithms, OTA update system, and communication protocols - all running entirely in software.

## Key Highlights

- ✅ No hardware required - 100% software simulation

- ✅ Runs on 8GB RAM systems - Uses < 50MB memory

- ✅ Production-quality C code - 4000+ lines with comprehensive testing

- ✅ Cross-platform - Windows, Linux, macOS compatible

- ✅ VS Code integration - Ready-to-use development environment

## Process

### Windows Installation

```
# Install MinGW-w64 (C compiler for Windows)
# Download from: https://github.com/msys2/msys2-installer/releases

# Add to PATH (PowerShell as Admin)
[Environment]::SetEnvironmentVariable("Path", "$env:Path;C:\msys64\mingw64\bin", "User")

# Install CMake from: https://cmake.org/download/

# Verify installation
gcc --version
cmake --version
```

### Build & Run

```
# Clone repository
git clone https://github.com/yourusername/EmbedRTOS.git
cd EmbedRTOS

# Build the project
make build

# Run the simulation
make run

# Or use VS Code: Press Ctrl+Shift+B to build, then F5 to run
```

###  Architecture

```
EmbedRTOS/
├── 📁 src/kernel/           # Custom RTOS Implementation
│   ├── scheduler.c         # Priority-based task scheduler
│   ├── queue.c             # Inter-task communication
│   └── semaphore.c         # Synchronization primitives
├── 📁 src/hal/             # Hardware Abstraction Layer
│   ├── hal.c              # Virtual GPIO, UART, ADC
│   └── virt_periph.c      # SPI, I2C, DMA, RTC simulation
├── 📁 src/algorithms/      # Signal Processing
│   └── kalman_filter.c    # Sensor fusion algorithms
├── 📁 src/ota/            # OTA Update System
│   ├── bootloader.c       # Dual-bank flash with rollback
│   └── ota_manager.c      # Update management
├── 📁 src/protocols/      # Communication
│   └── comm_protocol.c    # Custom binary protocol
├── 📁 src/app/            # Application Tasks
│   ├── sensor_task.c      # Sensor data collection
│   ├── comm_task.c        # Communication handling
│   └── monitor_task.c     # System monitoring
└── 📁 simulator/          # Virtual Hardware
    ├── virt_board.c       # Virtual MCU simulation
    └── visualization.c    # Real-time dashboard
```

### Features

#### Custom RTOS Scheduler

- Priority-based preemptive scheduling with 8 priority levels

- Task states: READY, RUNNING, BLOCKED, SUSPENDED

- Inter-task communication via queues and semaphores

- CPU usage monitoring with real-time statistics

- Task deadline monitoring with missed deadline tracking

```
// Example: Creating a task
scheduler_add_task(sensor_task, NULL, 1, 10, "Sensor");
// Priority 1, runs every 10ms
```

####  Hardware Abstraction Layer

- Virtual GPIO with input/output simulation

- UART emulation with baud rate configuration

- ADC simulation with 12-bit resolution

- SPI/I2C communication with timing simulation

- DMA controller with channel management

- RTC (Real-Time Clock) with calendar functions

#### Communication Protocol

- Custom binary protocol with start/end bytes

- CRC-16-CCITT error detection

- Sequence numbers for packet tracking

- Command/response architecture

- Support for OTA updates over simulated UART

#### OTA Update System

- Dual-bank flash memory (Active + Update slots)

- Firmware validation with CRC32 checks

- Rollback protection on update failure

- Progress tracking with percentage completion

- Signature verification (simulated)

### Demo

#### Interactive Dashboard

```
╔══════════════════════════════════════════════════════╗
║         EMBEDDED SENSOR HUB - LIVE DASHBOARD        ║
╠══════════════════════════════════════════════════════╣
║ System Status:                                      ║
║   Uptime: 150 s   CPU: 12.3%   Tasks: 4            ║
║   Memory: 48/256 KB (18.8%)                        ║
╠══════════════════════════════════════════════════════╣
║ Sensor Readings:                                    ║
║   Temperature: 25.3 °C   Humidity: 45.2%           ║
║   Pressure: 1013.2 hPa                             ║
╠══════════════════════════════════════════════════════╣
║ Commands: [s]tats  [d]ashboard  [u]pdate  [q]uit   ║
╚══════════════════════════════════════════════════════╝
```

#### OTA Update Simulation

```
# Simulate OTA update process
Press 'u' during simulation

# Update progress will show:
[OTA] Phase 1: Checking for updates...
[OTA] Phase 2: Downloading firmware (50%)...
[OTA] Phase 3: Validating signature...
[OTA] Phase 4: Update complete!
```

### System Requirements

- Minimum: 4GB RAM (for simulator only)

- Recommended: 8GB RAM (with VS Code/IDE)

- OS: Windows 10/11, Ubuntu 18.04+, macOS 10.15+

- Storage: 500MB free space

- Graphics: Terminal only (no GPU required)

