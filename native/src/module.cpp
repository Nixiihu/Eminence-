#include "module.hpp"

ModuleRegistry& ModuleRegistry::instance() {
    static ModuleRegistry registry;
    return registry;
}

std::string ModuleRegistry::key(const std::string& module, const std::string& setting) {
    return module + "\x1f" + setting;
}

void ModuleRegistry::initialize() {
    std::lock_guard lock(mutex_);
    if (!modules_.empty()) return;

    auto add = [this](const char* name, Category category, const char* description) {
        modules_.emplace(name, Module{name, category, description, false, 0});
    };

    const char* legit[] = {
        "Aimbot", "Anchor Macro", "Auto Clicker", "Auto Drain", "Auto Jump Reset",
        "Auto MLG", "Backtrack", "Bridge Assist", "Cobweb Key", "Crit Helper",
        "Fake Log", "Fast Exp", "Fluid Key", "Hitboxes", "Instant Shield",
        "Keep Sprint", "No Jump Delay"
    };
    const char* semiRage[] = {
        "Auto Cart Bow", "Auto Crystal", "Auto Elytra", "Auto Hit Crystal", "Auto Mace",
        "Auto Pearl Catch", "Auto Pot", "Auto Shield Breaker", "Auto Totem", "Auto Web",
        "Bow Auto Cart", "Bow Boost", "Breach Swap", "Diagonal Pearl Catch",
        "Double Anchor", "Elytra Bounce", "Elytra Wind Charge"
    };
    const char* misc[] = {
        "Anti Action", "Anti Bot", "Auto Armor", "Auto Pickaxe", "Auto Tool", "Cart Refill",
        "Chest Stealer", "Click Friend", "Crystal Optimizer", "Fast Place", "Friends", "Refill", "Sprint"
    };
    const char* visual[] = {
        "ESP", "Hit Effect", "HUD", "Item ESP", "No Bounce", "Optimization", "Storage ESP", "Toggle Notifications"
    };

    for (auto* n : legit) add(n, Category::Legit, "Legit utility");
    for (auto* n : semiRage) add(n, Category::SemiRage, "Semi-rage utility");
    for (auto* n : visual) add(n, Category::Visual, "Visual utility");
    for (auto* n : misc) add(n, Category::Misc, "Miscellaneous utility");

    auto b = [this](const char* m, const char* s, bool v){ boolSettings_[key(m,s)] = v; };
    auto f = [this](const char* m, const char* s, float v){ floatSettings_[key(m,s)] = v; };
    auto i = [this](const char* m, const char* s, int v){ intSettings_[key(m,s)] = v; };

    // Legit defaults
    f("Aimbot", "range", 6.0f); f("Aimbot", "smooth", 6.0f); b("Aimbot", "players", true); b("Aimbot", "mobs", true); b("Aimbot", "throughWalls", false);
    i("Anchor Macro", "minDelay", 0); i("Anchor Macro", "maxDelay", 50); b("Anchor Macro", "autoPlace", true); b("Anchor Macro", "autoExplode", true);
    i("Auto Clicker", "minCps", 8); i("Auto Clicker", "maxCps", 12);
    b("Auto Drain", "whileUsing", true);
    i("Auto Jump Reset", "cooldown", 4);
    f("Auto MLG", "minFallDistance", 5.0f); i("Auto MLG", "cooldown", 250);
    i("Backtrack", "ticks", 3); b("Backtrack", "targetPlayers", true);
    b("Bridge Assist", "autoSneak", true);
    b("Cobweb Key", "onlyWhenTargeting", true);
    b("Crit Helper", "onlyWhenFalling", true);
    b("Fake Log", "visualOnly", true);
    i("Fast Exp", "delay", 0);
    b("Fluid Key", "onlyOnBlock", true);
    f("Hitboxes", "scale", 1.0f);
    b("Instant Shield", "offhandOnly", true);
    b("Keep Sprint", "airborne", true);
    b("No Jump Delay", "holdJump", true);

    // Semi-Rage defaults
    f("Auto Cart Bow", "range", 12.0f); i("Auto Cart Bow", "delay", 120); b("Auto Cart Bow", "onlyWhenBowHeld", true);
    f("Auto Crystal", "range", 6.0f); i("Auto Crystal", "delay", 75); b("Auto Crystal", "onlyOwnPlacement", true);
    b("Auto Elytra", "autoEquip", true); i("Auto Elytra", "swapDelay", 2); b("Auto Elytra", "autoTakeoff", false);
    i("Auto Hit Crystal", "delay", 75); f("Auto Hit Crystal", "range", 6.0f);
    f("Auto Mace", "range", 6.0f); b("Auto Mace", "onlyFalling", true);
    b("Auto Pearl Catch", "onlyThrownPearls", true); i("Auto Pearl Catch", "delay", 50);
    i("Auto Pot", "healthThreshold", 10); i("Auto Pot", "delay", 100); b("Auto Pot", "preferSplash", true);
    i("Auto Shield Breaker", "delay", 75); b("Auto Shield Breaker", "onlyPlayers", true);
    b("Auto Totem", "offhand", true); i("Auto Totem", "healthThreshold", 8);
    i("Auto Web", "delay", 100); f("Auto Web", "range", 5.0f);
    i("Bow Auto Cart", "delay", 120); b("Bow Auto Cart", "onlyBowHeld", true);
    i("Bow Boost", "delay", 80); f("Bow Boost", "power", 1.0f);
    i("Breach Swap", "delay", 50); b("Breach Swap", "swapBack", true);
    f("Diagonal Pearl Catch", "range", 8.0f); i("Diagonal Pearl Catch", "delay", 50);
    i("Double Anchor", "delay", 50); b("Double Anchor", "autoPlace", true); b("Double Anchor", "autoExplode", true);
    i("Elytra Bounce", "cooldown", 250); f("Elytra Bounce", "boost", 1.0f);
    i("Elytra Wind Charge", "cooldown", 250); f("Elytra Wind Charge", "boost", 1.0f);

    // Misc defaults
    b("Friends", "showInHud", true); b("Friends", "preventTargeting", true);
    b("Sprint", "keepSprint", true); b("Sprint", "omniSprint", false);
    i("Fast Place", "delay", 0); b("Fast Place", "onlyBlocks", false);
    b("Refill", "hotbarOnly", true); i("Refill", "threshold", 1);
    b("Auto Tool", "switchToBest", true); b("Auto Tool", "includeSilkTouch", true);
    b("Auto Pickaxe", "onlyWhenMining", true); b("Auto Pickaxe", "swapBack", true);
    b("Auto Armor", "preferDurability", true); b("Auto Armor", "allowCursed", false); i("Auto Armor", "swapDelay", 2);
    i("Cart Refill", "threshold", 1); b("Cart Refill", "keepOne", true);
    i("Chest Stealer", "delay", 60); b("Chest Stealer", "takeAll", true); b("Chest Stealer", "closeAfter", true);
    f("Crystal Optimizer", "scanRadius", 6.0f); b("Crystal Optimizer", "prioritizeExisting", true);
    b("Anti Bot", "ignoreUnknownEntities", true); b("Anti Bot", "requirePlayerProfile", false);
    b("Anti Action", "blockUnwantedActions", true); b("Anti Action", "notification", true);
    b("Click Friend", "addOnClick", true); b("Click Friend", "removeOnShiftClick", true);

    // Visual defaults. The Java/Fabric side reads these through the JNI bridge.
    b("ESP", "boxes", true); b("ESP", "names", true); b("ESP", "health", true); b("ESP", "throughWalls", false); f("ESP", "range", 48.0f); f("ESP", "alpha", 0.70f);
    f("Hit Effect", "duration", 320.0f); f("Hit Effect", "scale", 1.0f); b("Hit Effect", "crosshairPulse", true); b("Hit Effect", "hurtFlash", true);
    b("HUD", "watermark", true); b("HUD", "coordinates", true); b("HUD", "fps", true); b("HUD", "direction", true); b("HUD", "armor", true);
    f("Item ESP", "range", 32.0f); b("Item ESP", "names", true); b("Item ESP", "boxes", true); f("Item ESP", "alpha", 0.65f);
    b("No Bounce", "viewBob", true); b("No Bounce", "damageTilt", true);
    b("Optimization", "hideClouds", true); b("Optimization", "hideWeather", false); b("Optimization", "reducedParticles", false); b("Optimization", "noEntityShadows", true);
    f("Storage ESP", "range", 32.0f); b("Storage ESP", "names", true); b("Storage ESP", "boxes", true); f("Storage ESP", "alpha", 0.55f);
    b("Toggle Notifications", "sound", false); i("Toggle Notifications", "duration", 1600);
}

std::vector<Module> ModuleRegistry::snapshot() const {
    std::lock_guard lock(mutex_);
    std::vector<Module> result; result.reserve(modules_.size());
    for (const auto& [_, module] : modules_) result.push_back(module);
    return result;
}

bool ModuleRegistry::setState(const std::string& name, bool enabled) {
    std::lock_guard lock(mutex_); auto it = modules_.find(name); if (it == modules_.end()) return false; it->second.enabled = enabled; return true;
}
bool ModuleRegistry::getState(const std::string& name) const {
    std::lock_guard lock(mutex_); auto it = modules_.find(name); return it != modules_.end() && it->second.enabled;
}
bool ModuleRegistry::setKey(const std::string& name, int keyCode) {
    std::lock_guard lock(mutex_); auto it = modules_.find(name); if (it == modules_.end()) return false; it->second.key = keyCode; return true;
}
int ModuleRegistry::getKey(const std::string& name) const {
    std::lock_guard lock(mutex_); auto it = modules_.find(name); return it == modules_.end() ? 0 : it->second.key;
}
bool ModuleRegistry::setBool(const std::string& module, const std::string& setting, bool value) {
    std::lock_guard lock(mutex_); if (!modules_.contains(module)) return false; boolSettings_[key(module, setting)] = value; return true;
}
bool ModuleRegistry::getBool(const std::string& module, const std::string& setting, bool fallback) const {
    std::lock_guard lock(mutex_); auto it = boolSettings_.find(key(module, setting)); return it == boolSettings_.end() ? fallback : it->second;
}
bool ModuleRegistry::setFloat(const std::string& module, const std::string& setting, float value) {
    std::lock_guard lock(mutex_); if (!modules_.contains(module)) return false; floatSettings_[key(module, setting)] = value; return true;
}
float ModuleRegistry::getFloat(const std::string& module, const std::string& setting, float fallback) const {
    std::lock_guard lock(mutex_); auto it = floatSettings_.find(key(module, setting)); return it == floatSettings_.end() ? fallback : it->second;
}
bool ModuleRegistry::setInt(const std::string& module, const std::string& setting, int value) {
    std::lock_guard lock(mutex_); if (!modules_.contains(module)) return false; intSettings_[key(module, setting)] = value; return true;
}
int ModuleRegistry::getInt(const std::string& module, const std::string& setting, int fallback) const {
    std::lock_guard lock(mutex_); auto it = intSettings_.find(key(module, setting)); return it == intSettings_.end() ? fallback : it->second;
}
