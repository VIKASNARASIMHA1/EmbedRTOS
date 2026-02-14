# 📑 Technical Design Rationale: EmbedRTOS

**Author:** Vikas Narasimha  
**Project:** Preemptive Real-Time Operating System (RTOS) Kernel  
**Date:** January 2026  

---

## 1. Problem Statement
In embedded systems, standard operating systems are too resource-heavy and lack the determinism required for time-critical applications. **EmbedRTOS** was designed as a lightweight, preemptive kernel that provides guaranteed task execution timing, efficient resource management, and a minimal memory footprint for microcontrollers.

---

## 2. Architectural Decisions & Trade-offs

### A. Preemptive vs. Cooperative Scheduling
* **Decision:** Implementation of a **Fixed-Priority Preemptive Scheduler**.
* **Rationale:** To ensure real-time determinism, the kernel must be able to suspend a lower-priority task immediately when a higher-priority task becomes ready. This is critical for handling urgent external interrupts or safety-critical logic.
* **Trade-off:** Preemption requires complex context-switching logic and careful management of shared resources to prevent race conditions.



### B. Context Switching & Stack Management
* **Decision:** Manual manipulation of the processor stack pointer and register preservation during interrupts.
* **Rationale:** By writing the context-switch routine in assembly or low-level C, the kernel minimizes "Switching Overhead." Each task is allocated its own isolated stack space, preventing memory corruption between processes.
* **Academic Significance:** This demonstrates a deep understanding of **Processor Architecture (ISA)** and the **Procedure Call Standard**.



### C. Inter-Process Communication (IPC)
* **Decision:** Implementation of **Semaphores and Mutexes** with priority awareness.
* **Rationale:** Tasks often need to synchronize or share hardware peripherals (like UART or SPI). The kernel provides synchronization primitives to ensure data integrity.
* **Refined Logic:** The design includes considerations for **Priority Inversion**, a common pitfall in RTOS design where a medium-priority task unintentionally blocks a high-priority one.

---

## 3. Kernel Services & Memory
To maintain a small "Code Size" (Footprint), EmbedRTOS utilizes:
1.  **Static Memory Allocation:** Task control blocks (TCBs) and stacks are defined at compile-time to avoid the non-deterministic behavior of the `heap` and `malloc()` in embedded environments.
2.  **Tick-less Idle (Optional):** Designed to put the CPU into low-power mode when no tasks are ready, significantly extending battery life for IoT applications.



---

## 4. Determinism and Latency
* **Interrupt Latency:** Optimized to ensure the time from hardware trigger to Task execution is minimized and constant (jitter-free).
* **System Tick:** A highly optimized timer ISR (Interrupt Service Routine) serves as the heartbeat of the system, managing task delays and timeouts.

---

## 5. Performance Characteristics
* **Kernel Overhead:** < 5% CPU utilization for system management.
* **Context Switch Time:** Measured in microseconds (depending on clock speed).
* **Footprint:** Fits within < 10KB of Flash memory, making it ideal for even the smallest ARM Cortex-M or AVR chips.

---

## 6. Conclusion
**EmbedRTOS** showcases proficiency in **Low-Level Systems Programming**, **Computer Architecture**, and **Resource-Constrained Development**. It proves that you can build the foundational software that interacts directly with hardware, providing a complete "Bottom-to-Top" view of modern computing in your portfolio.

---
