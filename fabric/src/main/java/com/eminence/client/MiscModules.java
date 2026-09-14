package com.eminence.client;

import java.util.*;

public final class MiscModules {
    public enum SettingType { BOOLEAN, INTEGER, DOUBLE, MODE }

    public static final class Module {
        public final String name;
        public boolean enabled;
        public int keybind;
        private final Map<String, Object> settings = new LinkedHashMap<>();

        Module(String name) { this.name = name; }

        public Module setting(String id, Object value) {
            settings.put(id, value);
            return this;
        }

        public Map<String, Object> settings() {
            return Collections.unmodifiableMap(settings);
        }

        public Object get(String id) { return settings.get(id); }

        public void set(String id, Object value) {
            if (settings.containsKey(id)) settings.put(id, value);
        }
    }

    private static final Map<String, Module> MODULES = new LinkedHashMap<>();

    static {
        add(new Module("Friends")
            .setting("showInHud", true).setting("preventTargeting", true));
        add(new Module("Sprint")
            .setting("keepSprint", true).setting("omniSprint", false));
        add(new Module("Fast Place")
            .setting("delay", 0).setting("onlyBlocks", false));
        add(new Module("Refill")
            .setting("hotbarOnly", true).setting("threshold", 1));
        add(new Module("Auto Tool")
            .setting("switchToBest", true).setting("includeSilkTouch", true));
        add(new Module("Auto Pickaxe")
            .setting("onlyWhenMining", true).setting("swapBack", true));
        add(new Module("Auto Armor")
            .setting("preferDurability", true).setting("allowCursed", false)
            .setting("swapDelay", 2));
        add(new Module("Cart Refill")
            .setting("minecartType", "Any").setting("keepOne", true)
            .setting("threshold", 1));
        add(new Module("Chest Stealer")
            .setting("delay", 60).setting("takeAll", true).setting("closeAfter", true));
        add(new Module("Crystal Optimizer")
            .setting("scanRadius", 6.0).setting("prioritizeExisting", true));
        add(new Module("Anti Bot")
            .setting("ignoreUnknownEntities", true).setting("requirePlayerProfile", false));
        add(new Module("Anti Action")
            .setting("blockUnwantedActions", true).setting("notification", true));
        add(new Module("Click Friend")
            .setting("addOnClick", true).setting("removeOnShiftClick", true));
    }

    private static void add(Module m) { MODULES.put(m.name, m); }

    public static Collection<Module> all() {
        return Collections.unmodifiableCollection(MODULES.values());
    }

    public static Module get(String name) { return MODULES.get(name); }

    public static void setEnabled(String name, boolean enabled) {
        Module m = MODULES.get(name);
        if (m != null) m.enabled = enabled;
    }

    /** Registers the category with the shared client lifecycle. Individual Minecraft-side
     *  behaviors should be added here only when they have a concrete 1.21.11 implementation. */
    public static void initialize() {
        // The native registry is the authoritative UI/state store. This method intentionally
        // does not duplicate module state, preventing the two GUIs from drifting apart.
    }

    private MiscModules() {}
}
