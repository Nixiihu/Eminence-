package com.eminence.client;

public final class NativeBridge {
    private static volatile boolean initialized;
    private static volatile boolean clientReady;

    private NativeBridge() {}

    public static void initialize() { initialized = true; }
    public static boolean isInitialized() { return initialized; }
    public static void setClientReady(boolean ready) { clientReady = ready; }
    public static boolean isClientReady() { return clientReady; }

    public static native boolean setModuleState(String module, boolean enabled);
    public static native boolean getModuleState(String module);
    public static native String[] getModules();
    public static native void setMenuOpen(boolean open);
    public static native boolean isMenuOpen();
    public static native boolean setBoolSetting(String module, String setting, boolean value);
    public static native boolean getBoolSetting(String module, String setting, boolean fallback);
    public static native boolean setFloatSetting(String module, String setting, float value);
    public static native float getFloatSetting(String module, String setting, float fallback);
    public static native boolean setIntSetting(String module, String setting, int value);
    public static native int getIntSetting(String module, String setting, int fallback);
}
