# Legit category implementation — Minecraft 1.21.11

The Legit category is now registered as its own category and is shared by both ImGui layouts.

Implemented modules:
- Aimbot — target selection + smooth rotation settings
- Anchor Macro — target-block use timing
- Auto Clicker — randomized CPS timing
- Auto Drain — automatic use while consuming the configured item
- Auto Jump Reset — falling jump reset timing
- Auto MLG — fall-distance trigger + item-use timing
- Backtrack — client-side position history buffer
- Bridge Assist — edge sneak assistance
- Cobweb Key — targeted item-use helper
- Crit Helper — falling attack timing
- Fake Log — local-only fake-log status state
- Fast Exp — repeated experience-item use
- Fluid Key — targeted fluid-item use
- Hitboxes — expanded target-selection bounds
- Instant Shield — automatic shield use
- Keep Sprint — sprint restoration
- No Jump Delay — immediate jump handling

Every module has its state, keybind, and settings in the same native `ModuleRegistry` used by both the dashboard and compact layouts. Changing a setting in one layout is immediately visible in the other.

Important: this source has not been launched in a real Minecraft 1.21.11 Windows runtime in this environment, so runtime compatibility still needs to be verified on the target machine before calling the build production-ready.
