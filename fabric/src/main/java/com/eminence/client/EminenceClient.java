package com.eminence.client;

import net.fabricmc.api.ClientModInitializer;
import net.fabricmc.fabric.api.client.event.lifecycle.v1.ClientLifecycleEvents;
import net.minecraft.resources.ResourceLocation;

public final class EminenceClient implements ClientModInitializer {
    public static ResourceLocation id(String path) { return ResourceLocation.fromNamespaceAndPath("eminence", path); }
    @Override
    public void onInitializeClient() {
        NativeBridge.initialize();

        LegitModules.initialize();
        SemiRageModules.initialize();
        VisualModules.initialize();
        MiscModules.initialize();

        ClientLifecycleEvents.CLIENT_STARTED.register(client -> {
            NativeBridge.setClientReady(true);
        });
    }
}
