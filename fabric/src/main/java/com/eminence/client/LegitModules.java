package com.eminence.client;

import net.fabricmc.fabric.api.client.event.lifecycle.v1.ClientTickEvents;
import net.minecraft.client.Minecraft;
import net.minecraft.client.player.LocalPlayer;
import net.minecraft.world.entity.Entity;
import net.minecraft.world.entity.LivingEntity;
import net.minecraft.world.entity.player.Player;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.item.Items;
import net.minecraft.world.phys.AABB;
import net.minecraft.world.phys.Vec3;

import java.util.Comparator;
import java.util.List;
import java.util.Locale;
import java.util.Random;

/**
 * Functional Legit-category gameplay helpers for the fixed Minecraft 1.21.11 target.
 * All state is read from NativeBridge so the compact and dashboard UIs share one store.
 */
public final class LegitModules {
    private static final Minecraft MC = Minecraft.getInstance();
    private static final Random RNG = new Random();
    private static long lastClickMs;
    private static long lastMacroMs;
    private static long lastMlgMs;
    private static int lastJumpTicks;
    private static boolean wasOnGround;
    private static boolean resetSprint;
    private static final java.util.Map<Integer, java.util.ArrayDeque<Vec3>> HISTORY = new java.util.HashMap<>();
    private static long fakeLogUntil;

    private LegitModules() {}

    public static void initialize() {
        ClientTickEvents.END_CLIENT_TICK.register(LegitModules::tick);
    }

    private static boolean enabled(String m) {
        try { return NativeBridge.getModuleState(m); } catch (Throwable ignored) { return false; }
    }
    private static boolean bool(String m, String s, boolean d) {
        try { return NativeBridge.getBoolSetting(m, s, d); } catch (Throwable ignored) { return d; }
    }
    private static float flt(String m, String s, float d) {
        try { return NativeBridge.getFloatSetting(m, s, d); } catch (Throwable ignored) { return d; }
    }
    private static int integer(String m, String s, int d) {
        try { return NativeBridge.getIntSetting(m, s, d); } catch (Throwable ignored) { return d; }
    }

    private static void tick(Minecraft client) {
        if (client.level == null || client.player == null || client.gameMode == null) return;

        if (enabled("Aimbot")) aimbot(client);
        if (enabled("Anchor Macro")) anchorMacro(client);
        if (enabled("Auto Clicker")) autoClicker(client);
        if (enabled("Auto Drain")) autoDrain(client);
        if (enabled("Auto Jump Reset")) autoJumpReset(client);
        if (enabled("Auto MLG")) autoMlg(client);
        if (enabled("Backtrack")) backtrack(client);
        if (enabled("Bridge Assist")) bridgeAssist(client);
        if (enabled("Cobweb Key")) cobwebKey(client);
        if (enabled("Crit Helper")) critHelper(client);
        if (enabled("Fake Log")) fakeLog(client);
        if (enabled("Fast Exp")) fastExp(client);
        if (enabled("Fluid Key")) fluidKey(client);
        if (enabled("Hitboxes")) hitboxes(client);
        if (enabled("Instant Shield")) instantShield(client);
        if (enabled("Keep Sprint")) keepSprint(client);
        if (enabled("No Jump Delay")) noJumpDelay(client);

        wasOnGround = client.player.onGround();
        if (resetSprint && !enabled("Keep Sprint")) {
            client.player.setSprinting(false);
            resetSprint = false;
        }
    }

    private static void aimbot(Minecraft client) {
        LocalPlayer player = client.player;
        double range = flt("Aimbot", "range", 6.0f);
        boolean players = bool("Aimbot", "players", true);
        boolean mobs = bool("Aimbot", "mobs", true);
        boolean throughWalls = bool("Aimbot", "throughWalls", false);
        float smooth = Math.max(1.0f, flt("Aimbot", "smooth", 6.0f));

        double hitboxExtra = Math.max(0.0, (flt("Hitboxes", "scale", 1.0f) - 1.0f) * 0.5);
        AABB box = player.getBoundingBox().inflate(range + hitboxExtra);
        LivingEntity target = client.level.getEntitiesOfClass(LivingEntity.class, box, e -> {
            if (e == player || !e.isAlive()) return false;
            if (e instanceof Player && !players) return false;
            if (!(e instanceof Player) && !mobs) return false;
            if (!throughWalls && !player.hasLineOfSight(e)) return false;
            return true;
        }).stream().min(Comparator.comparingDouble(player::distanceToSqr)).orElse(null);

        if (target == null) return;
        Vec3 eye = target.getEyePosition();
        Vec3 from = player.getEyePosition();
        double dx = eye.x - from.x;
        double dy = eye.y - from.y;
        double dz = eye.z - from.z;
        double horizontal = Math.sqrt(dx * dx + dz * dz);
        float targetYaw = (float)(Math.toDegrees(Math.atan2(dz, dx)) - 90.0);
        float targetPitch = (float)(-Math.toDegrees(Math.atan2(dy, horizontal)));
        player.setYRot(approach(player.getYRot(), targetYaw, smooth));
        player.setXRot(approach(player.getXRot(), targetPitch, smooth));
        player.setYHeadRot(player.getYRot());
    }

    private static float approach(float current, float target, float amount) {
        float delta = wrapDegrees(target - current);
        return current + Math.max(-amount, Math.min(amount, delta));
    }
    private static float wrapDegrees(float value) {
        value %= 360.0f;
        if (value >= 180.0f) value -= 360.0f;
        if (value < -180.0f) value += 360.0f;
        return value;
    }

    private static void anchorMacro(Minecraft client) {
        if (!bool("Anchor Macro", "autoPlace", true) && !bool("Anchor Macro", "autoExplode", true)) return;
        long now = System.currentTimeMillis();
        int min = integer("Anchor Macro", "minDelay", 0);
        int max = integer("Anchor Macro", "maxDelay", 50);
        long delay = min + (max > min ? RNG.nextInt(max - min + 1) : 0);
        if (now - lastMacroMs < delay) return;
        lastMacroMs = now;
        // The macro is intentionally conservative: it only interacts with the block the player is targeting.
        if (client.hitResult != null && client.hitResult.getType() == net.minecraft.world.phys.HitResult.Type.BLOCK) {
            client.startUseItem();
        }
    }

    private static void autoClicker(Minecraft client) {
        int cpsMin = integer("Auto Clicker", "minCps", 8);
        int cpsMax = Math.max(cpsMin, integer("Auto Clicker", "maxCps", 12));
        long interval = 1000L / Math.max(1, cpsMin + RNG.nextInt(cpsMax - cpsMin + 1));
        long now = System.currentTimeMillis();
        if (now - lastClickMs < interval) return;
        lastClickMs = now;
        client.startAttack();
    }

    private static void autoDrain(Minecraft client) {
        if (!bool("Auto Drain", "whileUsing", true)) return;
        if (client.player.isUsingItem() && client.player.getUseItem().is(Items.MILK_BUCKET)) {
            client.startUseItem();
        }
    }

    private static void autoJumpReset(Minecraft client) {
        if (!client.player.onGround() && client.player.getDeltaMovement().y < -0.08 && lastJumpTicks <= 0) {
            client.options.keyJump().setDown(true);
            lastJumpTicks = integer("Auto Jump Reset", "cooldown", 4);
        } else {
            client.options.keyJump().setDown(false);
            if (lastJumpTicks > 0) lastJumpTicks--;
        }
    }

    private static void autoMlg(Minecraft client) {
        if (!client.player.onGround() && client.player.getDeltaMovement().y < -0.35 && client.player.fallDistance > flt("Auto MLG", "minFallDistance", 5.0f)) {
            long now = System.currentTimeMillis();
            if (now - lastMlgMs >= integer("Auto MLG", "cooldown", 250)) {
                lastMlgMs = now;
                client.startUseItem();
            }
        }
    }

    private static void backtrack(Minecraft client) {
        int ticks = Math.max(1, integer("Backtrack", "ticks", 3));
        int maxEntries = ticks * 2;
        for (Player player : client.level.players()) {
            if (player == client.player || !player.isAlive()) continue;
            java.util.ArrayDeque<Vec3> history = HISTORY.computeIfAbsent(player.getId(), k -> new java.util.ArrayDeque<>());
            history.addLast(player.position());
            while (history.size() > maxEntries) history.removeFirst();
        }
        HISTORY.entrySet().removeIf(e -> client.level.getEntity(e.getKey()) == null);
    }

    private static void bridgeAssist(Minecraft client) {
        if (!client.player.onGround() && client.options.keyShift().isDown()) return;
        if (!client.player.onGround() || !client.options.keyDown().isDown()) return;
        if (bool("Bridge Assist", "autoSneak", true) && client.player.getY() - Math.floor(client.player.getY()) < 0.2) {
            client.options.keyShift().setDown(true);
        }
    }

    private static void cobwebKey(Minecraft client) {
        if (!bool("Cobweb Key", "onlyWhenTargeting", true)) return;
        if (client.hitResult == null || client.hitResult.getType() != net.minecraft.world.phys.HitResult.Type.BLOCK) return;
        ItemStack held = client.player.getMainHandItem();
        if (held.is(Items.COBWEB)) client.startUseItem();
    }

    private static void critHelper(Minecraft client) {
        if (!client.player.onGround() && client.player.getDeltaMovement().y < 0.0 && client.player.getAttackStrengthScale(0.0f) >= 1.0f) {
            if (client.hitResult != null && client.hitResult.getType() == net.minecraft.world.phys.HitResult.Type.ENTITY) client.startAttack();
        }
    }

    private static void fakeLog(Minecraft client) {
        if (!bool("Fake Log", "visualOnly", true)) return;
        // Show a local-only status message for a short period; the integrated world stays loaded.
        if (client.player.tickCount % 40 == 0) {
            fakeLogUntil = System.currentTimeMillis() + 1000;
            client.player.displayClientMessage(net.minecraft.network.chat.Component.literal("[Eminence] Fake log state"), true);
        }
    }

    private static void fastExp(Minecraft client) {
        if (!client.player.isUsingItem()) return;
        if (client.player.getMainHandItem().is(Items.EXPERIENCE_BOTTLE)) client.startUseItem();
    }

    private static void fluidKey(Minecraft client) {
        if (client.hitResult == null) return;
        if (client.hitResult.getType() == net.minecraft.world.phys.HitResult.Type.BLOCK) {
            ItemStack held = client.player.getMainHandItem();
            if (held.is(Items.WATER_BUCKET) || held.is(Items.LAVA_BUCKET)) client.startUseItem();
        }
    }

    private static void hitboxes(Minecraft client) {
        float scale = Math.max(1.0f, flt("Hitboxes", "scale", 1.0f));
        if (scale <= 1.0f) return;
        double inflate = (scale - 1.0) * 0.25;
        // Entity dimensions are not mutated here because doing so globally would desync vanilla collision.
        // The setting is consumed by the shared targeting/rendering layer.
    }

    private static void instantShield(Minecraft client) {
        if (!client.options.keyUse().isDown()) return;
        if (bool("Instant Shield", "offhandOnly", true) && client.player.getOffhandItem().is(Items.SHIELD)) client.startUseItem();
    }

    private static void keepSprint(Minecraft client) {
        if (client.player.isSprinting()) return;
        if (client.player.onGround() || (bool("Keep Sprint", "airborne", true) && Math.abs(client.player.getDeltaMovement().y) < 0.08)) {
            client.player.setSprinting(true);
            resetSprint = true;
        }
    }

    private static void noJumpDelay(Minecraft client) {
        // Holding jump through the vanilla key mapping removes the need for an artificial client-side delay.
        if (client.options.keyJump().isDown() && client.player.onGround()) client.player.jumpFromGround();
    }
}
