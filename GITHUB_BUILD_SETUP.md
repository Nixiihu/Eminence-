# GitHub build setup

This repository is prepared for the next build step.

Important:
- Target: Minecraft 1.21.11 / Fabric / Java 21.
- The GitHub workflow expects the Gradle wrapper at fabric/gradlew.
- The native C++/JNI component is Windows-specific and is not yet packaged into the Fabric JAR.
- Do not run a build until the Gradle wrapper and mappings configuration are corrected.
