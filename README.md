# Network-Enabled Firmware

Embedded firmware developed in C for an Industrial Internet of Things (IIoT) device. This repository demonstrates progressively building embedded functionality, beginning with low-level microcontroller peripherals and culminating in a simple network-enabled HTTP parser.

The project was completed as a series of cumulative modules, where each assignment introduced additional embedded systems concepts while building upon previous work.

---

## Repository Structure

```
network-enabled-firmware/
│
├── timer-interrupt/
├── eeprom/
├── watchdog-hysteresis/
└── iiot-http-parser/
```

### 1. Timer Interrupt & LED Blink Delay

Demonstrates fundamental embedded timing using hardware timers and interrupt service routines (ISRs).

Features include:

- Hardware timer configuration
- Interrupt-driven execution
- LED blinking without busy waiting
- Introduction to event-driven firmware

Concepts demonstrated:

- Interrupt Service Routines (ISRs)
- Timer peripherals
- Register-level programming
- Embedded timing

---

### 2. EEPROM Read/Write Buffer

Implements buffered non-volatile memory access using the MCU's EEPROM.

Features include:

- EEPROM read operations
- EEPROM write operations
- Buffered storage
- Persistent configuration data

Concepts demonstrated:

- Non-volatile memory
- Embedded storage
- Buffer management
- Data persistence

---

### 3. Watchdog Timer & Temperature Hysteresis

Adds system reliability and sensor-driven control logic.

Features include:

- Watchdog timer servicing
- Automatic recovery from firmware lockups
- Temperature monitoring
- Hysteresis-based state transitions

Concepts demonstrated:

- Fault tolerance
- Watchdog timers
- Embedded finite state machines
- Sensor processing
- Hysteresis control

---

### 4. IIoT HTTP Parser

The final module integrates previous functionality into a network-enabled firmware application capable of parsing incoming HTTP requests.

Features include:

- HTTP request parsing
- TCP socket communication
- Embedded state machines
- Network message processing
- Integration of previous firmware modules

Concepts demonstrated:

- Embedded networking
- HTTP protocol parsing
- Finite state machines
- Event-driven firmware architecture

---

## About the `iiot-http-parser` Directory

The `iiot-http-parser` directory represents the **final cumulative collection** in the project sequence.

Several of the header files and supporting libraries included in this directory originate from earlier modules in the course. They are included so that the final firmware can be built and executed as a complete application.

For that reason:

- The source files in the earlier directories show the incremental development of individual firmware components.
- The `iiot-http-parser` directory contains the integrated version used by the final implementation.
- Some supporting libraries and headers are intentionally left out in the final module because they provide functionality developed throughout the previous assignments.

This organization mirrors the progression of the coursework, where each new assignment extended the existing firmware rather than starting from scratch.

---

## Technologies

- C
- AVR Embedded Development
- Interrupts
- EEPROM
- Watchdog Timer
- TCP/IP Networking
- HTTP Parsing
- Finite State Machines
- Embedded Systems Programming

---

## Learning Objectives

This repository demonstrates experience with:

- Register-level embedded programming
- Interrupt-driven firmware design
- Persistent memory management
- Watchdog-based fault recovery
- Temperature monitoring and hysteresis
- Embedded networking
- HTTP protocol parsing
- Modular firmware architecture

---

## Notes

This repository is intended as a portfolio showcasing embedded systems concepts and firmware architecture. The projects are organized to reflect the incremental development process used throughout the course while preserving the final integrated implementation.
