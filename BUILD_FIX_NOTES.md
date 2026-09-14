# Corrected build setup

The previous build failed while resolving Loom's generated mappings:
loom:mappings:layered+hash.2198

This package normalizes the Fabric settings/repository configuration and keeps the
official Mojang mappings declaration. The Gradle wrapper must be generated after
the project configuration is verified.

Target: Minecraft 1.21.11 / Java 21 / Fabric.
