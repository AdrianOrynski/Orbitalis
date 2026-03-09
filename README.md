# Orbitalis – Orbital Mechanics Space Simulator

Orbitalis is a **3D space simulation project** developed in **Unreal Engine 5.7**.

The goal of the project is to create a small game-like simulator focused on **orbital mechanics and spacecraft control**.
The engine is used primarily for **rendering and visualization**, while **all physics calculations are implemented manually in C++**.

The project is developed as part of the course **Individual Programming Project**.

---

# Project Overview

Orbitalis combines elements of:

* space simulation
* realistic orbital mechanics
* precision spacecraft control

The player controls a spacecraft in different mission scenarios that require understanding **velocity, gravity and maneuvering in space**.

The project focuses on **simulation accuracy rather than arcade gameplay**.

---

# Missions

The simulator currently plans to include two mission types.

## Space Station Docking

The player must maneuver a spacecraft in orbit and successfully dock with a space station.

Challenges include:

* matching orbital velocity
* controlling relative motion
* performing precise thruster burns
* aligning with the docking port

---

## Lunar Lander (3D)

A modern 3D interpretation of the classic **Lunar Lander** concept.

The player must safely land a spacecraft on the Moon using limited thrust.

Landing success depends on:

* vertical velocity
* horizontal velocity
* orientation stability
* fuel management

---

# Author

**Orbitalis** was created by a third-year student of

**Applied Computer Science and Measurement Systems**
at the **Faculty of Physics and Astronomy, University of Wrocław**.

The project is developed as part of the course **Individual Programming Project**.

## Developer

| Role      | Avatar                                                       | Name               | GitHub                                     |
| --------- | ------------------------------------------------------------ | ------------------ | ------------------------------------------ |
| Developer | <img src="https://github.com/AdrianOrynski.png" width="40"/> | **Adrian Oryński** | [GitHub](https://github.com/AdrianOrynski) |

---

# Technology Stack

| Component       | Technology            |
| --------------- | --------------------- |
| Engine          | Unreal Engine 5.7     |
| Language        | C++                   |
| Rendering       | Unreal Engine         |
| Physics         | Custom implementation |
| Version Control | Git                   |
| Repository      | GitHub                |

---

# Simulation Architecture

The project separates **simulation logic** from the rendering engine.

Unreal Engine is responsible only for:

* 3D rendering
* scene management
* input handling

All physics calculations such as:

* velocity
* acceleration
* thrust
* gravity
* orbital motion

are implemented independently in C++.

This approach allows full control over the simulation model.

---

# Development Roadmap

## Week 1 — Project Setup

* Create Unreal Engine project
* Configure C++ project structure
* Setup Git repository
* Create basic 3D scene
* Implement camera controller

Goal:

* working project with controllable camera

---

## Week 2 — Physics Core

* Implement basic physics model
* Define object state:

  * position
  * velocity
  * acceleration
* Implement numerical integration

Goal:

* object moving in space using custom physics

---

## Week 3 — Orbital Mechanics

* Implement Newtonian gravity
* Simulate orbit around a planetary body
* Test stability of the simulation

Goal:

* stable orbital motion

---

## Week 4 — Spacecraft Controller

* Implement spacecraft thrusters
* Apply forces to the spacecraft
* Implement rotation and attitude control

Goal:

* controllable spacecraft in space

---

## Week 5 — Docking System

* Create space station object
* Implement docking detection
* Implement proximity and alignment checks

Goal:

* basic docking mechanic

---

## Week 6 — Docking Mission

* Create mission scenario
* Add success conditions
* Add failure conditions

Goal:

* playable docking mission

---

## Week 7 — Lunar Lander Physics

* Implement lunar gravity
* Implement thrust model
* Add fuel consumption system

Goal:

* spacecraft capable of controlled descent

---

## Week 8 — Lunar Lander Mission

* Create landing zone
* Implement landing conditions:

  * maximum vertical speed
  * maximum tilt

Goal:

* playable landing mission

---

## Week 9 — UI & Visualization

* Implement basic HUD
* Display information such as:

  * velocity
  * altitude
  * fuel level
* Improve camera controls

Goal:

* readable simulation interface

---

## Week 10 — Finalization

* bug fixing
* balancing physics parameters
* documentation
* preparing project presentation

Deliverables:

* working simulation
* GitHub repository
* project documentation
* demo gameplay

---

# Minimum Viable Product (MVP)

If development time becomes limited, the minimal version should include:

* custom physics engine
* controllable spacecraft
* orbital motion simulation
* docking mission

Optional features:

* lunar lander mission
* advanced HUD
* multiple celestial bodies
* improved visual effects

---