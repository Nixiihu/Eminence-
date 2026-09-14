package com.eminence.client;

import com.mojang.blaze3d.vertex.PoseStack;
import net.fabricmc.fabric.api.client.event.lifecycle.v1.ClientTickEvents;
import net.fabricmc.fabric.api.client.rendering.v1.world.WorldRenderEvents;
import net.fabricmc.fabric.api.client.rendering.v1.hud.HudElementRegistry;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiGraphics;
import net.minecraft.client.renderer.LevelRenderer;
import net.minecraft.client.renderer.MultiBufferSource;
import net.minecraft.client.renderer.RenderType;
import net.minecraft.client.renderer.entity.EntityRenderDispatcher;
import net.minecraft.core.BlockPos;
import net.minecraft.network.chat.Component;
import net.minecraft.world.entity.Entity;
import net.minecraft.world.entity.LivingEntity;
import net.minecraft.world.entity.item.ItemEntity;
import net.minecraft.world.level.block.entity.BlockEntity;
import net.minecraft.world.level.block.entity.ChestBlockEntity;
import net.minecraft.world.level.block.entity.EnderChestBlockEntity;
import net.minecraft.world.level.block.entity.HopperBlockEntity;
import net.minecraft.world.level.block.entity.ShulkerBoxBlockEntity;
import net.minecraft.world.level.block.entity.BarrelBlockEntity;
import net.minecraft.world.level.block.entity.DispenserBlockEntity;
import net.minecraft.world.level.block.entity.DropperBlockEntity;
import net.minecraft.world.level.block.entity.FurnaceBlockEntity;
import net.minecraft.world.phys.AABB;
import net.minecraft.world.phys.Vec3;
import org.joml.Matrix4f;

import java.util.ArrayDeque;
import java.util.Deque;
import java.util.Locale;

/**
 * Functional, single-player visual layer for the fixed 1.21.11 target.
 * Rendering is kept on Fabric's render events rather than native OpenGL hooks,
 * which keeps the visual modules compatible with the game's renderer.
 */
public final class VisualModules {
    private static final Minecraft MC = Minecraft.getInstance();
    private static final int ACCENT = 0xFFB46CFF;
    private static final Deque<Notification> NOTIFICATIONS = new ArrayDeque<>();
    private static final boolean[] LAST = new boolean[8];
    private static long lastPoll;
    private static boolean capturedOptions;
    private static boolean originalBobView, originalEntityShadows;
    private static net.minecraft.client.option.CloudRenderMode originalClouds;
    private static net.minecraft.particle.ParticlesMode originalParticles;

    private VisualModules() {}

    public static void initialize() {
        // NativeBridge is registered by the native side. Until then, all reads
        // safely return defaults instead of breaking the Fabric client.
        HudElementRegistry.addLast(EminenceClient.id("visual_hud"), VisualModules::renderHud);
        HudElementRegistry.addLast(EminenceClient.id("visual_hit_effect"), VisualModules::renderHitEffect);
        WorldRenderEvents.AFTER_ENTITIES.register(VisualModules::renderWorld);
        ClientTickEvents.END_CLIENT_TICK.register(VisualModules::tick);
    }

    private static boolean enabled(String module) {
        try { return NativeBridge.getModuleState(module); }
        catch (Throwable ignored) { return false; }
    }
    private static boolean bool(String m, String s, boolean d) {
        try { return NativeBridge.getBoolSetting(m,s,d); } catch (Throwable ignored) { return d; }
    }
    private static float flt(String m, String s, float d) {
        try { return NativeBridge.getFloatSetting(m,s,d); } catch (Throwable ignored) { return d; }
    }
    private static int integer(String m, String s, int d) {
        try { return NativeBridge.getIntSetting(m,s,d); } catch (Throwable ignored) { return d; }
    }

    private static void tick(Minecraft client) {
        if (client.level == null || client.player == null) return;
        if (!capturedOptions) {
            originalBobView = client.options.getBobView().get();
            originalEntityShadows = client.options.getEntityShadows().get();
            originalClouds = client.options.getCloudRenderMode().get();
            originalParticles = client.options.particles().get();
            capturedOptions = true;
        }
        long now = System.currentTimeMillis();
        if (now - lastPoll < 50) return;
        lastPoll = now;

        String[] names = {"ESP","Hit Effect","HUD","Item ESP","No Bounce","Optimization","Storage ESP","Toggle Notifications"};
        for (int i=0;i<names.length;i++) {
            boolean state = enabled(names[i]);
            if (state != LAST[i] && enabled("Toggle Notifications")) {
                NOTIFICATIONS.addLast(new Notification(names[i] + (state ? " enabled" : " disabled"), now + integer("Toggle Notifications","duration",1600)));
            }
            LAST[i] = state;
        }

        if (enabled("No Bounce")) {
            if (bool("No Bounce","viewBob",true)) client.options.getBobView().set(false);
            if (bool("No Bounce","damageTilt",true)) client.options.getDamageTiltStrength().set(0.0);
        } else if (capturedOptions) {
            client.options.getBobView().set(originalBobView);
        }
        if (enabled("Optimization")) {
            if (bool("Optimization","hideClouds",true)) client.options.getCloudRenderMode().set(net.minecraft.client.option.CloudRenderMode.OFF);
            if (bool("Optimization","reducedParticles",false)) client.options.particles().set(net.minecraft.particle.ParticlesMode.MINIMAL);
            if (bool("Optimization","noEntityShadows",true)) client.options.getEntityShadows().set(false);
        } else if (capturedOptions) {
            client.options.getCloudRenderMode().set(originalClouds);
            client.options.particles().set(originalParticles);
            client.options.getEntityShadows().set(originalEntityShadows);
        }
    }

    private static void renderHud(GuiGraphics g, net.minecraft.client.DeltaTracker delta) {
        if (!enabled("HUD") || MC.player == null || MC.level == null) return;
        int x = 10, y = 10;
        if (bool("HUD","watermark",true)) {
            g.fill(x-6,y-5,x+118,y+20,0xA00B0B12);
            g.fill(x-6,y-5,x-3,y+20,ACCENT);
            g.drawString(MC.font, Component.literal("EMINENCE"), x, y, ACCENT, true);
        }
        int line = 30;
        if (bool("HUD","fps",true)) { g.drawString(MC.font, Component.literal("FPS  " + MC.getFps()), x, line, 0xFFE7E7EF, true); line += 11; }
        if (bool("HUD","coordinates",true)) {
            String xyz = String.format(Locale.ROOT,"XYZ  %.1f  %.1f  %.1f",MC.player.getX(),MC.player.getY(),MC.player.getZ());
            g.drawString(MC.font, Component.literal(xyz), x, line, 0xFFE7E7EF, true); line += 11;
        }
        if (bool("HUD","direction",true)) {
            g.drawString(MC.font, Component.literal("FACING  " + MC.player.getDirection().getName().toUpperCase(Locale.ROOT)), x, line, 0xFFE7E7EF, true); line += 11;
        }
        if (bool("HUD","armor",true)) {
            g.drawString(MC.font, Component.literal("ARMOR  " + armorValue() + "%"), x, line, 0xFFE7E7EF, true);
        }
    }

    private static int armorValue() {
        int total=0,max=0;
        for (var stack: MC.player.getArmorSlots()) { total += stack.getDamageValue() == 0 ? stack.getMaxDamage() : stack.getMaxDamage()-stack.getDamageValue(); max += stack.getMaxDamage(); }
        return max == 0 ? 0 : Math.max(0, Math.min(100,(total*100)/max));
    }

    private static void renderHitEffect(GuiGraphics g, net.minecraft.client.DeltaTracker delta) {
        if (!enabled("Hit Effect") || MC.player == null) return;
        if (bool("Hit Effect","hurtFlash",true) && MC.player.hurtTime > 0) {
            float a = Math.min(0.28f, MC.player.hurtTime / 10.0f);
            int alpha = ((int)(a*255)) << 24;
            g.fill(0,0,g.guiWidth(),g.guiHeight(),alpha | 0xB02028);
        }
        if (bool("Hit Effect","crosshairPulse",true) && MC.player.attackAnim > 0.0f) {
            float p = Math.max(0.0f, 1.0f - MC.player.attackAnim);
            int cx=g.guiWidth()/2, cy=g.guiHeight()/2, size=(int)(3 + p*5*flt("Hit Effect","scale",1));
            g.fill(cx-size,cy-1,cx+size+1,cy+1,ACCENT);
            g.fill(cx-1,cy-size,cx+1,cy+size+1,ACCENT);
        }
        drawNotifications(g);
    }

    private static void drawNotifications(GuiGraphics g) {
        long now=System.currentTimeMillis(); int y=g.guiHeight()-30;
        while(!NOTIFICATIONS.isEmpty() && NOTIFICATIONS.peekFirst().until < now) NOTIFICATIONS.removeFirst();
        for(Notification n:NOTIFICATIONS){ int w=MC.font.width(n.text)+22; int x=g.guiWidth()-w-12; g.fill(x,y-3,x+w,y+17,0xD90B0B12); g.fill(x,y-3,x+2,y+17,ACCENT); g.drawString(MC.font,Component.literal(n.text),x+8,y+2,0xFFF4F4FA,true); y-=24; }
    }

    private static void renderWorld(net.fabricmc.fabric.api.client.rendering.v1.world.WorldRenderContext ctx) {
        if (MC.level == null || MC.player == null) return;
        if (enabled("ESP")) renderEntityBoxes(ctx);
        if (enabled("Item ESP")) renderItemBoxes(ctx);
        if (enabled("Storage ESP")) renderStorageBoxes(ctx);
    }

    private static void renderEntityBoxes(net.fabricmc.fabric.api.client.rendering.v1.world.WorldRenderContext ctx) {
        float range=flt("ESP","range",48); double max=range*range;
        MultiBufferSource consumers=ctx.consumers(); var vc=consumers.getBuffer(RenderType.lines()); PoseStack ps=ctx.matrices(); Vec3 cam=MC.gameRenderer.getMainCamera().getPosition();
        for(Entity e:MC.level.entitiesForRendering()) {
            if (!(e instanceof LivingEntity living) || e==MC.player || e.isInvisible() || e.distanceToSqr(MC.player)>max) continue;
            AABB box=e.getBoundingBox().inflate(0.025).move(-cam.x,-cam.y,-cam.z);
            int rgb=living.hurtTime>0?0xFF3CFF68:0xFFB46CFF;
            float r=((rgb>>16)&255)/255f,g=((rgb>>8)&255)/255f,b=(rgb&255)/255f;
            LevelRenderer.renderLineBox(ps,vc,box,r,g,b,flt("ESP","alpha",0.70f));
        }
    }

    private static void renderItemBoxes(net.fabricmc.fabric.api.client.rendering.v1.world.WorldRenderContext ctx) {
        float range=flt("Item ESP","range",32); double max=range*range; MultiBufferSource c=ctx.consumers(); var vc=c.getBuffer(RenderType.lines()); PoseStack ps=ctx.matrices(); Vec3 cam=MC.gameRenderer.getMainCamera().getPosition();
        for(Entity e:MC.level.entitiesForRendering()) if(e instanceof ItemEntity item && e.distanceToSqr(MC.player)<=max) {
            AABB box=item.getBoundingBox().inflate(0.05).move(-cam.x,-cam.y,-cam.z); LevelRenderer.renderLineBox(ps,vc,box,0.25f,0.9f,1.0f,flt("Item ESP","alpha",0.65f));
        }
    }

    private static void renderStorageBoxes(net.fabricmc.fabric.api.client.rendering.v1.world.WorldRenderContext ctx) {
        float range=flt("Storage ESP","range",32); double max=range*range; MultiBufferSource c=ctx.consumers(); var vc=c.getBuffer(RenderType.lines()); PoseStack ps=ctx.matrices(); Vec3 cam=MC.gameRenderer.getMainCamera().getPosition();
        for(BlockEntity be:MC.level.getBlockEntities().values()) {
            if(!isStorage(be)) continue; BlockPos p=be.getBlockPos(); if(p.distSqr(MC.player.blockPosition())>max) continue;
            AABB box=new AABB(p).move(-cam.x,-cam.y,-cam.z); LevelRenderer.renderLineBox(ps,vc,box,1.0f,0.62f,0.2f,flt("Storage ESP","alpha",0.55f));
        }
    }

    private static boolean isStorage(BlockEntity be) {
        return be instanceof ChestBlockEntity || be instanceof EnderChestBlockEntity || be instanceof ShulkerBoxBlockEntity || be instanceof BarrelBlockEntity || be instanceof HopperBlockEntity || be instanceof DispenserBlockEntity || be instanceof DropperBlockEntity || be instanceof FurnaceBlockEntity;
    }

    private record Notification(String text,long until) {}
}
