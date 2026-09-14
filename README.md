# Eminence Client — Minecraft 1.21.11

Fixed target: Minecraft Java 1.21.11 / Fabric, Windows x64.

## Visuals pass

The first implementation pass now focuses on the complete Visuals category:

- ESP — distance-limited living-entity boxes with configurable range, alpha and filters.
- Hit Effect — hurt flash and attack/crosshair pulse with duration/scale controls.
- HUD — watermark, FPS, coordinates, facing direction and armor percentage.
- Item ESP — distance-limited dropped-item boxes.
- No Bounce — reversible view-bobbing and damage-tilt suppression.
- Optimization — reversible cloud, particle and entity-shadow reductions.
- Storage ESP — distance-limited boxes for common storage/block containers.
- Toggle Notifications — animated enable/disable notifications for visual modules.

Every Visuals module is connected to the same native module state/settings store, so the C++ dashboard and Fabric renderer use the same values.

## GUI

There are two switchable layouts:

1. Dashboard — category sidebar, module list and Anchor-Macro-style settings pane in one GUI.
2. Compact — all four categories displayed together in the module overlay.

Visual modules expose module enable state, individual keybinds and module-specific controls from the settings pane. Accent presets and Rainbow mode remain live.

## Build

### Fabric bridge

From `fabric`:

```text
gradlew.bat build
```

### Native library

From a Visual Studio Developer PowerShell:

```text
cd native
cmake -S . -B build -A x64
cmake --build build --config Release
```

Requirements: Windows 10/11 x64, JDK 21, Visual Studio 2022 Desktop C++, CMake 3.24+, Minecraft Java 1.21.11, Fabric Loader 0.18.1 and Fabric API 0.141.1+1.21.11.

## Verification note

The source was updated against the 1.21.11 Fabric/Mojang API references, but this Linux environment does not contain the Windows toolchain or a Gradle installation, so a real Windows compile/gameplay run cannot honestly be claimed here. The next verification step is to build on Windows and run each Visuals module in a local single-player world, checking enable/disable, keybinds, settings changes, render stability and module restoration.
\n\n## Final audit notes\n- Both GUI layouts use the same native module registry/state store.\n- All four categories are exposed in both layouts.\n- Dashboard now includes module search.\n- Default menu key is Right Shift and can be changed at runtime.\n- Misc settings are exposed in the full settings pane and Misc is initialized with the client.\n- Runtime Minecraft behavior still requires a real 1.21.11 client test; source inspection alone cannot prove every gameplay module.\n