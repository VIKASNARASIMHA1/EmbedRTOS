# EmbedRTOS
EmbedRTOS is a comprehensive embedded systems simulator designed to implement a complete embedded ecosystem including a custom RTOS, hardware abstraction layer, sensor fusion algorithms, OTA update system, and communication protocols - all running entirely in software.

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

## Development

### Building from Source

```
# Method 1: Using Makefile (Recommended)
make build      # Standard build
make debug      # Build with debug symbols
make release    # Optimized build
make clean      # Clean build files

# Method 2: Using CMake directly
mkdir build && cd build
cmake ..        # Linux/macOS
cmake -G "MinGW Makefiles" ..  # Windows
make -j4

# Method 3: Quick build (no CMake)
make quick      # Direct compilation
```

### Running Tests

```
# Run all tests
make test

# Expected output:
╔══════════════════════════════════════════════════════╗
║                   TEST SUMMARY                       ║
╠══════════════════════════════════════════════════════╣
║ Total Tests:  8                                      ║
║ Passed:       8                                      ║
║ Failed:       0                                      ║
║ Success Rate: 100.0%                                 ║
║ Status:       ✅ ALL TESTS PASSED                    ║
╚══════════════════════════════════════════════════════╝
```

## Code Structure Deep Dive

### RTOS Scheduler

```
// Key data structures
typedef struct {
    void (*task_func)(void*);
    uint32_t priority;
    uint32_t period_ms;
    char name[16];
    uint32_t run_count;
} task_t;

// Scheduling algorithm
task_t* find_highest_priority_task(void) {
    // Implements priority-based selection
    // with round-robin for same priority
}
```

### Kalman Filter

```
// Kalman filter update step
float kalman1d_update(kalman_t* kf, float measurement) {
    kf->p = kf->p + kf->q;                    // Prediction
    kf->k = kf->p / (kf->p + kf->r);          // Kalman gain
    kf->x = kf->x + kf->k * (measurement - kf->x); // Update
    kf->p = (1.0f - kf->k) * kf->p;           // Covariance update
    return kf->x;
}
```

### OTA Bootloader

```
// Firmware validation
bootloader_error_t validate_firmware(uint32_t address) {
    // Check magic number
    // Verify CRC32
    // Validate size
    // Check entry point
    // Return validation status
}
```

## Skills Demonstrated

### Embedded Systems Engineering

- Real-Time Operating Systems: Custom scheduler design, task management

- Hardware Abstraction: Virtual peripheral simulation, register-level emulation

- Interrupt Handling: Simulated interrupt service routines

- Memory Management: Flash emulation, memory-mapped I/O

- Power Management: Sleep modes, power state simulation

### Algorithms & Data Structures

- Kalman Filtering: State estimation, noise reduction

- Circular Buffers: Efficient data handling

- CRC Algorithms: Error detection implementation

- Priority Queues: Task scheduling

- Binary Protocols: Data packing/unpacking

### Software Architecture

- Modular Design: Clean separation of concerns

- Layered Architecture: HAL, drivers, application layers

- Interface Design: Clean APIs between modules

- Error Handling: Comprehensive error detection/recovery

- Testing Strategy: Unit, integration, system tests

###  Development Practices

- Build Systems: CMake, Makefile automation

- Cross-Platform: Windows/Linux/macOS compatibility

- Version Control: Git with proper commit history

- Documentation: Comprehensive README, code comments

- Code Quality: Consistent style, error checking\

### Development Workflow

```
# 1. Fork the repository
# 2. Create a feature branch
git checkout -b feature/amazing-feature

# 3. Make your changes
# 4. Run tests
make test

# 5. Commit changes
git commit -m "Add amazing feature"

# 6. Push to branch
git push origin feature/amazing-feature

# 7. Create Pull Request
```

## Acknowledgments

- Inspiration: Real-world embedded systems and RTOS implementations

- Tools: GCC, CMake, VS Code, and the open-source community

- Testing: All contributors who helped test on various platforms

- Community: Embedded systems developers sharing knowledge online

##  Get Started Now!

```
# Clone and run in under 2 minutes
git clone https://github.com/yourusername/EmbedRTOS.git
cd EmbedRTOS

# Build (30 seconds)
make build

# Run the simulator
make run

# Explore the code
code .  # Opens in VS Code with full configuration
```
## License

```
MIT License

Copyright (c) 2024 Vikas Narasimha

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```
