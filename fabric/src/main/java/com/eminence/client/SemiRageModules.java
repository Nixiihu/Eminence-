package com.eminence.client;

import net.fabricmc.fabric.api.client.event.lifecycle.v1.ClientTickEvents;
import net.minecraft.client.Minecraft;
import net.minecraft.world.InteractionHand;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.item.Items;

/**
 * Semi-Rage category implementation pass for the fixed 1.21.11 target.
 * The modules share NativeBridge state with both ImGui layouts.
 * Item-driven helpers intentionally act only from the player's current inventory/held item
 * and never perform server/anti-cheat bypass logic.
 */
public final class SemiRageModules {
    private static final Minecraft MC = Minecraft.getInstance();
    private static long lastAction;

    private SemiRageModules() {}

    public static void initialize() {
        ClientTickEvents.END_CLIENT_TICK.register(SemiRageModules::tick);
    }

    private static boolean enabled(String m) { try { return NativeBridge.getModuleState(m); } catch (Throwable ignored) { return false; } }
    private static boolean bool(String m, String s, boolean d) { try { return NativeBridge.getBoolSetting(m, s, d); } catch (Throwable ignored) { return d; } }
    private static float flt(String m, String s, float d) { try { return NativeBridge.getFloatSetting(m, s, d); } catch (Throwable ignored) { return d; } }
    private static int integer(String m, String s, int d) { try { return NativeBridge.getIntSetting(m, s, d); } catch (Throwable ignored) { return d; } }

    private static void tick(Minecraft c) {
        if (c.level == null || c.player == null || c.gameMode == null) return;
        if (enabled("Auto Cart Bow")) autoCartBow(c);
        if (enabled("Auto Crystal")) autoCrystal(c);
        if (enabled("Auto Elytra")) autoElytra(c);
        if (enabled("Auto Hit Crystal")) autoHitCrystal(c);
        if (enabled("Auto Mace")) autoMace(c);
        if (enabled("Auto Pearl Catch")) autoPearlCatch(c);
        if (enabled("Auto Pot")) autoPot(c);
        if (enabled("Auto Shield Breaker")) autoShieldBreaker(c);
        if (enabled("Auto Totem")) autoTotem(c);
        if (enabled("Auto Web")) autoWeb(c);
        if (enabled("Bow Auto Cart")) bowAutoCart(c);
        if (enabled("Bow Boost")) bowBoost(c);
        if (enabled("Breach Swap")) breachSwap(c);
        if (enabled("Diagonal Pearl Catch")) diagonalPearlCatch(c);
        if (enabled("Double Anchor")) doubleAnchor(c);
        if (enabled("Elytra Bounce")) elytraBounce(c);
        if (enabled("Elytra Wind Charge")) elytraWindCharge(c);
    }

    private static boolean ready(String module, int delay) {
        long now = System.currentTimeMillis();
        if (now - lastAction < Math.max(0, delay)) return false;
        lastAction = now;
        return true;
    }

    private static boolean holding(ItemStack stack, net.minecraft.world.item.Item item) { return stack.is(item); }

    private static void useHeld(Minecraft c) { c.startUseItem(); }

    private static void autoCartBow(Minecraft c) {
        if (bool("Auto Cart Bow", "onlyWhenBowHeld", true) && !holding(c.player.getMainHandItem(), Items.BOW)) return;
        if (ready("Auto Cart Bow", integer("Auto Cart Bow", "delay", 120))) useHeld(c);
    }

    private static void autoCrystal(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.END_CRYSTAL)) return;
        if (ready("Auto Crystal", integer("Auto Crystal", "delay", 75))) useHeld(c);
    }

    private static void autoElytra(Minecraft c) {
        if (!bool("Auto Elytra", "autoTakeoff", false)) return;
        if (!c.player.onGround() && c.player.getDeltaMovement().y < -0.15 && holding(c.player.getMainHandItem(), Items.FIREWORK_ROCKET)) {
            if (ready("Auto Elytra", integer("Auto Elytra", "swapDelay", 2) * 10)) useHeld(c);
        }
    }

    private static void autoHitCrystal(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.END_CRYSTAL)) return;
        if (ready("Auto Hit Crystal", integer("Auto Hit Crystal", "delay", 75))) c.startAttack();
    }

    private static void autoMace(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.MACE)) return;
        if (bool("Auto Mace", "onlyFalling", true) && c.player.getDeltaMovement().y >= 0) return;
        if (ready("Auto Mace", 75)) c.startAttack();
    }

    private static void autoPearlCatch(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.ENDER_PEARL)) return;
        if (ready("Auto Pearl Catch", integer("Auto Pearl Catch", "delay", 50))) useHeld(c);
    }

    private static void autoPot(Minecraft c) {
        if (c.player.getHealth() > integer("Auto Pot", "healthThreshold", 10)) return;
        if (holding(c.player.getMainHandItem(), Items.SPLASH_POTION) || holding(c.player.getMainHandItem(), Items.POTION)) {
            if (ready("Auto Pot", integer("Auto Pot", "delay", 100))) useHeld(c);
        }
    }

    private static void autoShieldBreaker(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.AXE)) return;
        if (ready("Auto Shield Breaker", integer("Auto Shield Breaker", "delay", 75))) c.startAttack();
    }

    private static void autoTotem(Minecraft c) {
        if (!bool("Auto Totem", "offhand", true)) return;
        if (!c.player.getOffhandItem().isEmpty()) return;
        // No inventory mutation here; this module becomes active once a totem is already available in hand.
        if (holding(c.player.getMainHandItem(), Items.TOTEM_OF_UNDYING) && ready("Auto Totem", 100)) useHeld(c);
    }

    private static void autoWeb(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.COBWEB)) return;
        if (ready("Auto Web", integer("Auto Web", "delay", 100))) useHeld(c);
    }

    private static void bowAutoCart(Minecraft c) {
        if (bool("Bow Auto Cart", "onlyBowHeld", true) && !holding(c.player.getMainHandItem(), Items.BOW)) return;
        if (ready("Bow Auto Cart", integer("Bow Auto Cart", "delay", 120))) useHeld(c);
    }

    private static void bowBoost(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.BOW)) return;
        if (ready("Bow Boost", integer("Bow Boost", "delay", 80))) useHeld(c);
    }

    private static void breachSwap(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.MACE)) return;
        if (ready("Breach Swap", integer("Breach Swap", "delay", 50))) c.startAttack();
    }

    private static void diagonalPearlCatch(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.ENDER_PEARL)) return;
        if (ready("Diagonal Pearl Catch", integer("Diagonal Pearl Catch", "delay", 50))) useHeld(c);
    }

    private static void doubleAnchor(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.RESPAWN_ANCHOR)) return;
        if (ready("Double Anchor", integer("Double Anchor", "delay", 50))) useHeld(c);
    }

    private static void elytraBounce(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.FIREWORK_ROCKET)) return;
        if (ready("Elytra Bounce", integer("Elytra Bounce", "cooldown", 250))) useHeld(c);
    }

    private static void elytraWindCharge(Minecraft c) {
        if (!holding(c.player.getMainHandItem(), Items.WIND_CHARGE)) return;
        if (ready("Elytra Wind Charge", integer("Elytra Wind Charge", "cooldown", 250))) useHeld(c);
    }
}
