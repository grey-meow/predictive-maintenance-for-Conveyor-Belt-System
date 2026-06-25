# Predictive Maintenance for Conveyor Belt System using ESP32 & FreeRTOS

![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange)
![FreeRTOS](https://img.shields.io/badge/FreeRTOS-v10-blue)
![Wokwi](https://img.shields.io/badge/Simulation-Wokwi-green)
![Language](https://img.shields.io/badge/Language-C%2B%2B-blue)

An ESP32-based predictive maintenance system developed using **FreeRTOS**, **PlatformIO**, and **Wokwi**. The project demonstrates real-time task scheduling, queue communication, semaphore synchronization, mutex protection, runtime statistics, and deadline monitoring for industrial conveyor belt monitoring.

---

# Course Information

| Item           | Details                                                                     |
| -------------- | --------------------------------------------------------------------------- |
| **Course**     | BERC4833 & BERC4834 Real Time System                                        |
| **Programme**  | Bachelor of Computer Engineering Technology (Computer Systems) with Honours |
| **University** | Universiti Teknikal Malaysia Melaka (UTeM)                                  |
| **Semester**   | Semester 2, Session 2025/2026                                               |

---

# Project Overview

This project implements a predictive maintenance node for a conveyor belt system using the ESP32 microcontroller and FreeRTOS.

The system periodically performs:

* Temperature monitoring every **200 ms**
* Vibration monitoring every **500 ms**
* SD/UART logging every **1000 ms**
* Runtime statistics collection
* LED heartbeat indication

The implementation demonstrates multitasking, priority scheduling, synchronization, and inter-task communication within a real-time embedded environment.

---

# Problem Statement

Industrial predictive maintenance systems require multiple periodic tasks to execute concurrently while satisfying strict timing constraints.

A simulated SD-card logging operation requires **300 ms**, potentially introducing timing jitter and causing deadline violations for higher-priority tasks.

To solve this issue, the project applies:

* FreeRTOS Preemptive Scheduling
* Rate Monotonic Scheduling (RMS)
* Queue-based communication
* Binary Semaphore synchronization
* Mutex protection

These techniques ensure that all critical sensing tasks continue meeting their timing requirements while lower-priority logging operations execute in the background.

---

# Objectives

* Develop a multitasking embedded system using ESP32 and FreeRTOS.
* Implement periodic temperature and vibration monitoring.
* Simulate slow SD-card logging.
* Demonstrate Queue, Semaphore and Mutex synchronization.
* Monitor runtime performance and deadline misses.
* Validate scheduling behaviour using Wokwi simulation.

---

# System Architecture

## Tasks Implemented

| Task                    | Function                       | Period    | Priority    |
| ----------------------- | ------------------------------ | --------- | ----------- |
| Temperature Task        | Read temperature sensor        | 200 ms    | 5 (Highest) |
| Vibration Task          | Read vibration sensor          | 500 ms    | 4           |
| Log Trigger Task        | Trigger logger using semaphore | 1000 ms   | 3           |
| Logger Task             | Simulate SD/UART logging       | Triggered | 2           |
| Runtime Statistics Task | Display runtime statistics     | 2000 ms   | 1           |
| LED Alive Task          | Blink heartbeat LED            | 500 ms    | 1           |

---

## Scheduling Method

* FreeRTOS Preemptive Scheduler
* Rate Monotonic Scheduling (RMS)
* Fixed-priority scheduling
* Periodic execution using `vTaskDelayUntil()`

---

# Hardware Components

* ESP32 DevKit V4
* NTC Temperature Sensor
* Potentiometer (Vibration Sensor Simulation)
* Red LED
* 330 Ω Resistor
* Logic Analyzer
* Serial Monitor

---

# Pin Assignment

| Component          | GPIO   |
| ------------------ | ------ |
| Temperature Sensor | GPIO34 |
| Vibration Sensor   | GPIO35 |
| LED                | GPIO2  |
| Logic Analyzer D0  | GPIO13 |
| Logic Analyzer D1  | GPIO12 |
| Logic Analyzer D2  | GPIO14 |
| Logic Analyzer D3  | GPIO27 |

---

# Project Structure

```text
.
├── src
│   └── main.cpp
│
├── include
├── lib
├── test
│
├── diagram.json
├── platformio.ini
├── wokwi.toml
├── .gitignore
├── README.md
│
└── screenshots
```

---

# FreeRTOS Features Demonstrated

* Task Creation
* Task Priorities
* Rate Monotonic Scheduling (RMS)
* Periodic Tasks
* Queue Communication
* Binary Semaphore
* Mutex Protection
* Runtime Statistics
* Deadline Monitoring
* Jitter Measurement
* Queue Overflow Detection
* Inter-task Communication

---

# Expected Runtime Output

```text
Predictive Maintenance FreeRTOS Demo Started

Temperature Task : 200 ms
Vibration Task  : 500 ms
Logger Task     : 1000 ms
LED Task        : 2 Hz

------ SD/UART LOGGING START ------
Logger triggered at: 2510 ms

Time(ms)    Sensor  ADC   Interval  Jitter
1709        TEMP    1370  200 ms    0 ms
2010        VIB     4095  500 ms    0 ms
2509        TEMP    1750  200 ms    0 ms
2510        VIB     4095  500 ms    0 ms

------ SD/UART LOGGING END --------

[STATS] Runtime Statistics

Temperature samples      : 10
Vibration samples        : 4
Max temperature jitter   : 0 ms
Max vibration jitter     : 0 ms
Temperature deadline miss: 0
Queue records dropped    : 0
Queue spaces available   : 26
```

---

# Getting Started

## Requirements

* Visual Studio Code
* PlatformIO Extension
* Wokwi Extension

---

## Clone Repository

```bash
git clone https://github.com/grey-meow/predictive-maintenance-for-Conveyor-Belt-System.git
```

---

## Build Project

```bash
pio run
```

---

## Upload to ESP32

```bash
pio run --target upload
```

---

## Open Serial Monitor

```bash
pio device monitor
```

---

## Run Wokwi Simulation

1. Open the project in Visual Studio Code.
2. Build the project using PlatformIO.
3. Start the Wokwi Simulator.
4. Open the Serial Monitor.
5. Observe:

   * Temperature readings every 200 ms.
   * Vibration readings every 500 ms.
   * Logger execution every 1000 ms.
   * Runtime statistics every 2000 ms.
   * LED heartbeat blinking at 2 Hz.

---

# Team Members

| Name                                | Student ID |
| ----------------------------------- | ---------- |
| NUR HUDA HIDAYAH BINTI IRMAN JOHAN  | B122310435 |
| NURDINI IZZATI BINTI NORAZMI        | B122310053 |
| NUR SARAH DHAMIRAH BINTI HAMDAN     | B122310361 |
| SYAHIDATUL FARHANAH BINTI MAZHAIZAL | B122310450 |
| NURUL FATIHA BINTI NOR AZMI         | B122310380 |

---

# Repository Contents

* ESP32 FreeRTOS source code
* PlatformIO project configuration
* Wokwi simulation files
* Queue implementation
* Semaphore synchronization
* Mutex protection
* Runtime statistics
* Project documentation

---

# References

1. Wang, J. *Real-Time Embedded Systems*. John Wiley & Sons, 2017.
2. Liu, C. L., & Layland, J. W. S. *Scheduling Algorithms for Multiprogramming in a Hard Real-Time Environment*. Journal of the ACM, 1973.
3. Liu, J. W. S. *Real-Time Systems*. Pearson Education, 2006.

---

## License

This repository is developed for academic purposes as part of the **BERC4833 & BERC4834 Real Time System** course at **Universiti Teknikal Malaysia Melaka (UTeM)**.
