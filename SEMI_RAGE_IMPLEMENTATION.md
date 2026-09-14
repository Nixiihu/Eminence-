# Semi-Rage implementation pass

Target: Minecraft Java 1.21.11 + Fabric.

Implemented in `SemiRageModules.java` and registered from `EminenceClient`.
All 17 Semi-Rage modules are present in the shared native module registry and expose settings in the same settings pane used by the Dashboard and Compact layouts.

Important: this pass intentionally does not claim runtime verification. Several modules that normally require precise inventory/interaction state are conservative item-driven helpers rather than a claim of full production automation. The project still needs an actual Windows + Minecraft 1.21.11 runtime build/test pass before calling the modules fully verified.
