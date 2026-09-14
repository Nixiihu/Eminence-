#include "imgui_menu.hpp"
#include "module.hpp"

#include <windows.h>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>\n#include <cctype>

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_opengl3.h"

namespace {
    bool initialized = false, menuOpen = false, settingsOpen = false, configsOpen = false;
    bool waitingForKey = false, waitingForMenuKey = false, dashboard = true;
    std::string keyTarget;\n    int menuKey = VK_RSHIFT;\n    char searchBuffer[128] = {};
    std::string selectedModule = "Aimbot";
    Category selectedCategory = Category::Legit;
    HWND gameWindow = nullptr;
    WNDPROC originalWndProc = nullptr;

    enum class AccentMode { Preset, Rainbow };
    AccentMode accentMode = AccentMode::Preset;
    int accentPreset = 0;
    struct AccentPreset { const char* name; ImVec4 color; };
    constexpr AccentPreset presets[] = {
        {"Purple", {0.43f,0.33f,0.82f,1}}, {"Lime", {0.45f,0.90f,0.20f,1}},
        {"Cyan", {0.15f,0.78f,0.95f,1}}, {"Red", {0.95f,0.20f,0.25f,1}},
        {"Orange", {1.00f,0.52f,0.16f,1}}, {"Pink", {0.95f,0.30f,0.70f,1}}
    };

    ImVec4 accentColor() {
        if (accentMode == AccentMode::Rainbow) {
            float t = static_cast<float>(ImGui::GetTime()) * 0.20f;
            return ImColor::HSV(std::fmod(t,1.0f),0.78f,0.95f,1.0f);
        }
        return presets[accentPreset].color;
    }

    void applyTheme() {
        const ImVec4 a = accentColor();
        ImGuiStyle& s = ImGui::GetStyle();
        s.WindowRounding = 10; s.ChildRounding = 8; s.FrameRounding = 6; s.PopupRounding = 7;
        s.ScrollbarRounding = 6; s.GrabRounding = 6; s.WindowPadding = {14,14}; s.ItemSpacing = {8,8};
        s.Colors[ImGuiCol_WindowBg] = {0.045f,0.047f,0.060f,0.98f};
        s.Colors[ImGuiCol_ChildBg] = {0.065f,0.068f,0.085f,0.92f};
        s.Colors[ImGuiCol_FrameBg] = {0.09f,0.095f,0.12f,1};
        s.Colors[ImGuiCol_FrameBgHovered] = {a.x,a.y,a.z,0.22f};
        s.Colors[ImGuiCol_Header] = {a.x,a.y,a.z,0.28f};
        s.Colors[ImGuiCol_HeaderHovered] = {a.x,a.y,a.z,0.42f};
        s.Colors[ImGuiCol_HeaderActive] = {a.x,a.y,a.z,0.55f};
        s.Colors[ImGuiCol_Button] = {a.x,a.y,a.z,0.32f};
        s.Colors[ImGuiCol_ButtonHovered] = {a.x,a.y,a.z,0.50f};
        s.Colors[ImGuiCol_ButtonActive] = {a.x,a.y,a.z,0.68f};
        s.Colors[ImGuiCol_CheckMark] = a; s.Colors[ImGuiCol_SliderGrab] = a; s.Colors[ImGuiCol_SliderGrabActive] = a;
    }

    const char* categoryName(Category c) {
        switch(c){ case Category::Legit:return "Legit"; case Category::SemiRage:return "Semi-Rage"; case Category::Visual:return "Visual"; case Category::Misc:return "Misc"; }
        return "";
    }
    std::string keyName(int key) {
        if (!key) return "Unbound";
        UINT scan=MapVirtualKeyA(static_cast<UINT>(key),MAPVK_VK_TO_VSC); LONG lp=static_cast<LONG>(scan<<16); char name[128]{};
        return GetKeyNameTextA(lp,name,sizeof(name))>0 ? std::string(name) : "Key "+std::to_string(key);
    }

    void drawKeybind(Module& m) {
        ImGui::PushID((m.name+"_key").c_str()); ImGui::SameLine();
        if (waitingForKey && keyTarget==m.name) ImGui::SmallButton("Press key...");
        else if (ImGui::SmallButton(keyName(m.key).c_str())) { waitingForKey=true; keyTarget=m.name; }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Click to bind. Escape clears the bind.");
        ImGui::PopID();
    }

    void drawModuleRow(Module m) {
        ImGui::PushID(m.name.c_str());
        bool enabled=m.enabled;
        if (ImGui::Checkbox(m.name.c_str(),&enabled)) ModuleRegistry::instance().setState(m.name,enabled);
        drawKeybind(m);
        if (ImGui::IsItemHovered() && !m.description.empty()) ImGui::SetTooltip("%s",m.description.c_str());
        if (selectedModule==m.name && ImGui::IsItemHovered()) ImGui::SetItemDefaultFocus();
        ImGui::PopID();
    }

    void drawFloat(const char* label,const char* mod,const char* setting,float min,float max,const char* fmt="%.2f") {
        float v=ModuleRegistry::instance().getFloat(mod,setting,min);
        if(ImGui::SliderFloat(label,&v,min,max,fmt)) ModuleRegistry::instance().setFloat(mod,setting,v);
    }
    void drawBool(const char* label,const char* mod,const char* setting,bool fallback) {
        bool v=ModuleRegistry::instance().getBool(mod,setting,fallback);
        if(ImGui::Checkbox(label,&v)) ModuleRegistry::instance().setBool(mod,setting,v);
    }
    void drawInt(const char* label,const char* mod,const char* setting,int min,int max) {
        int v=ModuleRegistry::instance().getInt(mod,setting,min);
        if(ImGui::SliderInt(label,&v,min,max)) ModuleRegistry::instance().setInt(mod,setting,v);
    }

    void drawVisualSettings(const std::string& m) {
        ImGui::TextDisabled("VISUAL SETTINGS"); ImGui::Separator();
        if(m=="ESP") {
            drawFloat("Range","ESP","range",8,96,"%.0f blocks"); drawFloat("Alpha","ESP","alpha",0.15f,1.0f,"%.2f");
            drawBool("Boxes","ESP","boxes",true); drawBool("Names","ESP","names",true); drawBool("Health bars","ESP","health",true); drawBool("Through walls","ESP","throughWalls",false);
        } else if(m=="Hit Effect") {
            drawFloat("Duration","Hit Effect","duration",100,1200,"%.0f ms"); drawFloat("Scale","Hit Effect","scale",0.5f,2.0f,"%.2fx");
            drawBool("Crosshair pulse","Hit Effect","crosshairPulse",true); drawBool("Hurt flash","Hit Effect","hurtFlash",true);
        } else if(m=="HUD") {
            drawBool("Watermark","HUD","watermark",true); drawBool("Coordinates","HUD","coordinates",true); drawBool("FPS","HUD","fps",true); drawBool("Direction","HUD","direction",true); drawBool("Armor","HUD","armor",true);
        } else if(m=="Item ESP") {
            drawFloat("Range","Item ESP","range",8,64,"%.0f blocks"); drawFloat("Alpha","Item ESP","alpha",0.15f,1.0f,"%.2f"); drawBool("Boxes","Item ESP","boxes",true); drawBool("Names","Item ESP","names",true);
        } else if(m=="No Bounce") {
            drawBool("View bobbing","No Bounce","viewBob",true); drawBool("Damage tilt","No Bounce","damageTilt",true);
        } else if(m=="Optimization") {
            drawBool("Hide clouds","Optimization","hideClouds",true); drawBool("Hide weather (reserved)","Optimization","hideWeather",false); drawBool("Reduced particles","Optimization","reducedParticles",false); drawBool("No entity shadows","Optimization","noEntityShadows",true);
        } else if(m=="Storage ESP") {
            drawFloat("Range","Storage ESP","range",8,64,"%.0f blocks"); drawFloat("Alpha","Storage ESP","alpha",0.15f,1.0f,"%.2f"); drawBool("Boxes","Storage ESP","boxes",true); drawBool("Names","Storage ESP","names",true);
        } else if(m=="Toggle Notifications") {
            drawInt("Duration","Toggle Notifications","duration",500,5000); drawBool("Sound","Toggle Notifications","sound",false);
        }
    }

    void drawLegitSettings(const std::string& m) {
        ImGui::TextDisabled("LEGIT SETTINGS"); ImGui::Separator();
        if(m=="Aimbot") {
            drawFloat("Range","Aimbot","range",1,12,"%.1f blocks");
            drawFloat("Smooth","Aimbot","smooth",1,20,"%.1f deg/tick");
            drawBool("Players","Aimbot","players",true); drawBool("Mobs","Aimbot","mobs",true); drawBool("Through walls","Aimbot","throughWalls",false);
        } else if(m=="Anchor Macro") {
            drawInt("Min delay","Anchor Macro","minDelay",0,250); drawInt("Max delay","Anchor Macro","maxDelay",0,500);
            drawBool("Auto place","Anchor Macro","autoPlace",true); drawBool("Auto explode","Anchor Macro","autoExplode",true);
        } else if(m=="Auto Clicker") {
            drawInt("Min CPS","Auto Clicker","minCps",1,20); drawInt("Max CPS","Auto Clicker","maxCps",1,30);
        } else if(m=="Auto Drain") drawBool("While using","Auto Drain","whileUsing",true);
        else if(m=="Auto Jump Reset") drawInt("Cooldown","Auto Jump Reset","cooldown",1,12);
        else if(m=="Auto MLG") { drawFloat("Min fall distance","Auto MLG","minFallDistance",2,30,"%.1f blocks"); drawInt("Cooldown","Auto MLG","cooldown",50,1000); }
        else if(m=="Backtrack") { drawInt("History","Backtrack","ticks",1,10); drawBool("Target players","Backtrack","targetPlayers",true); }
        else if(m=="Bridge Assist") drawBool("Auto sneak","Bridge Assist","autoSneak",true);
        else if(m=="Cobweb Key") drawBool("Only when targeting","Cobweb Key","onlyWhenTargeting",true);
        else if(m=="Crit Helper") drawBool("Only when falling","Crit Helper","onlyWhenFalling",true);
        else if(m=="Fake Log") drawBool("Visual only","Fake Log","visualOnly",true);
        else if(m=="Fast Exp") drawInt("Delay","Fast Exp","delay",0,250);
        else if(m=="Fluid Key") drawBool("Only on block","Fluid Key","onlyOnBlock",true);
        else if(m=="Hitboxes") drawFloat("Scale","Hitboxes","scale",1,2,"%.2fx");
        else if(m=="Instant Shield") drawBool("Offhand only","Instant Shield","offhandOnly",true);
        else if(m=="Keep Sprint") drawBool("Airborne","Keep Sprint","airborne",true);
        else if(m=="No Jump Delay") drawBool("Hold jump","No Jump Delay","holdJump",true);
    }

    void drawSemiRageSettings(const std::string& m) {
        ImGui::TextDisabled("SEMI-RAGE SETTINGS"); ImGui::Separator();
        if(m=="Auto Cart Bow") { drawFloat("Range","Auto Cart Bow","range",4,32,"%.0f blocks"); drawInt("Delay","Auto Cart Bow","delay",0,500); drawBool("Only when bow held","Auto Cart Bow","onlyWhenBowHeld",true); }
        else if(m=="Auto Crystal") { drawFloat("Range","Auto Crystal","range",2,12,"%.1f blocks"); drawInt("Delay","Auto Crystal","delay",0,500); drawBool("Only own placement","Auto Crystal","onlyOwnPlacement",true); }
        else if(m=="Auto Elytra") { drawBool("Auto equip","Auto Elytra","autoEquip",true); drawInt("Swap delay","Auto Elytra","swapDelay",0,20); drawBool("Auto takeoff","Auto Elytra","autoTakeoff",false); }
        else if(m=="Auto Hit Crystal") { drawFloat("Range","Auto Hit Crystal","range",2,12,"%.1f blocks"); drawInt("Delay","Auto Hit Crystal","delay",0,500); }
        else if(m=="Auto Mace") { drawFloat("Range","Auto Mace","range",2,12,"%.1f blocks"); drawBool("Only falling","Auto Mace","onlyFalling",true); }
        else if(m=="Auto Pearl Catch") { drawBool("Only thrown pearls","Auto Pearl Catch","onlyThrownPearls",true); drawInt("Delay","Auto Pearl Catch","delay",0,500); }
        else if(m=="Auto Pot") { drawInt("Health threshold","Auto Pot","healthThreshold",1,20); drawInt("Delay","Auto Pot","delay",0,500); drawBool("Prefer splash","Auto Pot","preferSplash",true); }
        else if(m=="Auto Shield Breaker") { drawInt("Delay","Auto Shield Breaker","delay",0,500); drawBool("Only players","Auto Shield Breaker","onlyPlayers",true); }
        else if(m=="Auto Totem") { drawBool("Offhand","Auto Totem","offhand",true); drawInt("Health threshold","Auto Totem","healthThreshold",1,20); }
        else if(m=="Auto Web") { drawFloat("Range","Auto Web","range",2,10,"%.1f blocks"); drawInt("Delay","Auto Web","delay",0,500); }
        else if(m=="Bow Auto Cart") { drawInt("Delay","Bow Auto Cart","delay",0,500); drawBool("Only bow held","Bow Auto Cart","onlyBowHeld",true); }
        else if(m=="Bow Boost") { drawInt("Delay","Bow Boost","delay",0,500); drawFloat("Power","Bow Boost","power",0.1f,3.0f,"%.1fx"); }
        else if(m=="Breach Swap") { drawInt("Delay","Breach Swap","delay",0,500); drawBool("Swap back","Breach Swap","swapBack",true); }
        else if(m=="Diagonal Pearl Catch") { drawFloat("Range","Diagonal Pearl Catch","range",2,16,"%.1f blocks"); drawInt("Delay","Diagonal Pearl Catch","delay",0,500); }
        else if(m=="Double Anchor") { drawInt("Delay","Double Anchor","delay",0,500); drawBool("Auto place","Double Anchor","autoPlace",true); drawBool("Auto explode","Double Anchor","autoExplode",true); }
        else if(m=="Elytra Bounce") { drawInt("Cooldown","Elytra Bounce","cooldown",50,1000); drawFloat("Boost","Elytra Bounce","boost",0.1f,3.0f,"%.1fx"); }
        else if(m=="Elytra Wind Charge") { drawInt("Cooldown","Elytra Wind Charge","cooldown",50,1000); drawFloat("Boost","Elytra Wind Charge","boost",0.1f,3.0f,"%.1fx"); }
    }

    void drawMiscSettings(const std::string& m) {
        ImGui::TextDisabled("MISC SETTINGS"); ImGui::Separator();
        if(m=="Friends"){ drawBool("Show in HUD","Friends","showInHud",true); drawBool("Prevent targeting","Friends","preventTargeting",true); }
        else if(m=="Sprint"){ drawBool("Keep sprint","Sprint","keepSprint",true); drawBool("Omni sprint","Sprint","omniSprint",false); }
        else if(m=="Fast Place"){ drawInt("Delay","Fast Place","delay",0,10); drawBool("Only blocks","Fast Place","onlyBlocks",false); }
        else if(m=="Refill"){ drawBool("Hotbar only","Refill","hotbarOnly",true); drawInt("Threshold","Refill","threshold",1,9); }
        else if(m=="Auto Tool"){ drawBool("Switch to best","Auto Tool","switchToBest",true); drawBool("Include Silk Touch","Auto Tool","includeSilkTouch",true); }
        else if(m=="Auto Pickaxe"){ drawBool("Only when mining","Auto Pickaxe","onlyWhenMining",true); drawBool("Swap back","Auto Pickaxe","swapBack",true); }
        else if(m=="Auto Armor"){ drawBool("Prefer durability","Auto Armor","preferDurability",true); drawBool("Allow cursed","Auto Armor","allowCursed",false); drawInt("Swap delay","Auto Armor","swapDelay",0,20); }
        else if(m=="Cart Refill"){ drawInt("Threshold","Cart Refill","threshold",1,9); drawBool("Keep one","Cart Refill","keepOne",true); }
        else if(m=="Chest Stealer"){ drawInt("Delay","Chest Stealer","delay",0,500); drawBool("Take all","Chest Stealer","takeAll",true); drawBool("Close after","Chest Stealer","closeAfter",true); }
        else if(m=="Crystal Optimizer"){ drawFloat("Scan radius","Crystal Optimizer","scanRadius",1,12,"%.1f blocks"); drawBool("Prioritize existing","Crystal Optimizer","prioritizeExisting",true); }
        else if(m=="Anti Bot"){ drawBool("Ignore unknown entities","Anti Bot","ignoreUnknownEntities",true); drawBool("Require player profile","Anti Bot","requirePlayerProfile",false); }
        else if(m=="Anti Action"){ drawBool("Block unwanted actions","Anti Action","blockUnwantedActions",true); drawBool("Notification","Anti Action","notification",true); }
        else if(m=="Click Friend"){ drawBool("Add on click","Click Friend","addOnClick",true); drawBool("Remove on shift click","Click Friend","removeOnShiftClick",true); }
        else { ImGui::TextDisabled("No additional settings."); }
    }

    void drawSettingsPane() {
        Module target{}; bool found=false;
        for(auto m:ModuleRegistry::instance().snapshot()) if(m.name==selectedModule){target=m;found=true;break;}
        if(!found) return;
        ImGui::Text("%s",target.name.c_str()); ImGui::SameLine(); ImGui::TextDisabled("%s",categoryName(target.category));
        ImGui::Separator();
        bool en=target.enabled; if(ImGui::Checkbox("Enabled",&en)) ModuleRegistry::instance().setState(target.name,en);
        drawKeybind(target);
        if(target.category==Category::Visual) drawVisualSettings(target.name);
        else if(target.category==Category::Legit) drawLegitSettings(target.name);
        else if(target.category==Category::SemiRage) drawSemiRageSettings(target.name);
        else if(target.category==Category::Misc) drawMiscSettings(target.name);
    }

    void drawDashboard() {
        ImGui::BeginChild("Sidebar",ImVec2(170,0),true);
        ImGui::TextUnformatted("EMINENCE"); ImGui::TextDisabled("1.21.11"); ImGui::Separator();
        const Category cats[]={Category::Legit,Category::SemiRage,Category::Visual,Category::Misc};
        for(Category c:cats){ bool active=c==selectedCategory; if(active) ImGui::PushStyleColor(ImGuiCol_Button,accentColor()); if(ImGui::Button(categoryName(c),{-1,38})) selectedCategory=c; if(active) ImGui::PopStyleColor(); }
        ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("Modules",ImVec2(300,0),true);
        ImGui::Text("%s",categoryName(selectedCategory));
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##module_search", "Search modules...", searchBuffer, IM_ARRAYSIZE(searchBuffer));
        ImGui::Separator();
        std::string query = searchBuffer;
        std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        for(auto m:ModuleRegistry::instance().snapshot()) if(m.category==selectedCategory){
            std::string lower = m.name;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
            if(!query.empty() && lower.find(query)==std::string::npos) continue;
            bool active=m.name==selectedModule; if(active) ImGui::PushStyleColor(ImGuiCol_Header,ImVec4(accentColor().x,accentColor().y,accentColor().z,0.22f));
            if(ImGui::Selectable(m.name.c_str(),active,0,ImVec2(0,34))) selectedModule=m.name;
            ImGui::SameLine(ImGui::GetContentRegionAvail().x+ImGui::GetCursorPosX()-26); ImGui::TextDisabled(m.enabled?"ON":"");
            if(active) ImGui::PopStyleColor();
        }
        ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("ModuleSettings",ImVec2(0,0),true); drawSettingsPane(); ImGui::EndChild();
    }

    void drawOverlay() {
        ImGui::SetNextItemWidth(360);
        ImGui::InputTextWithHint("##compact_search", "Search modules...", searchBuffer, IM_ARRAYSIZE(searchBuffer));
        std::string query = searchBuffer;
        std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        if(ImGui::BeginTable("categories",4,ImGuiTableFlags_SizingStretchSame)){
            const Category cats[]={Category::Legit,Category::SemiRage,Category::Visual,Category::Misc}; const char* names[]={"Legit","Semi-Rage","Visual","Misc"};
            for(int i=0;i<4;i++){ ImGui::TableNextColumn(); ImGui::BeginChild(names[i],ImVec2(0,0),true); ImGui::TextUnformatted(names[i]); ImGui::Separator(); for(auto m:ModuleRegistry::instance().snapshot()) if(m.category==cats[i]) {
                std::string lower=m.name;
                std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
                if(!query.empty() && lower.find(query)==std::string::npos) continue;
                drawModuleRow(m);
            } ImGui::EndChild(); }
            ImGui::EndTable();
        }
    }

    void drawSettings() {
        ImGui::BeginChild("SettingsPanel",ImVec2(0,0),true);
        ImGui::TextUnformatted("Interface"); ImGui::Separator();
        const char* modes[]={"Preset","Rainbow"}; int mode=accentMode==AccentMode::Rainbow; if(ImGui::Combo("Accent mode",&mode,modes,2)) accentMode=mode?AccentMode::Rainbow:AccentMode::Preset;
        if(accentMode==AccentMode::Preset){ const char* names[6]; for(int i=0;i<6;i++)names[i]=presets[i].name; ImGui::Combo("Accent",&accentPreset,names,6); }
        for(int i=0;i<6;i++){ ImGui::PushID(i); if(i)ImGui::SameLine(); if(ImGui::ColorButton(presets[i].name,presets[i].color,0,{24,24})){accentMode=AccentMode::Preset;accentPreset=i;} ImGui::PopID(); }
        ImGui::Separator(); if(waitingForMenuKey){ ImGui::Text("Press a key for the menu..."); }
        else { ImGui::Text("Menu key: %s", menuKey==VK_RSHIFT?"Right Shift":"Custom"); }
        if(!waitingForMenuKey && ImGui::Button("Change Menu Key")) waitingForMenuKey=true;
        ImGui::SameLine(); if(ImGui::Button("Reset to Right Shift")) { menuKey=VK_RSHIFT; waitingForMenuKey=false; } ImGui::Text("Dashboard: %s",dashboard?"ON":"OFF"); ImGui::TextDisabled("Legit, Semi-Rage, Visual and Misc share one module/settings registry across both layouts.");
        ImGui::EndChild();
    }
}

namespace ImGuiMenu {
    bool initialize(HWND hwnd){
        if(initialized||!hwnd)return initialized; gameWindow=hwnd; ModuleRegistry::instance().initialize();
        IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGui::GetIO().IniFilename=nullptr; applyTheme();
        if(!ImGui_ImplWin32_Init(hwnd))return false; if(!ImGui_ImplOpenGL3_Init("#version 130")){ImGui_ImplWin32_Shutdown();return false;}
        originalWndProc=reinterpret_cast<WNDPROC>(SetWindowLongPtrW(hwnd,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(&wndProc))); initialized=true; return true;
    }
    void shutdown(){ if(!initialized)return; if(gameWindow&&originalWndProc)SetWindowLongPtrW(gameWindow,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(originalWndProc)); ImGui_ImplOpenGL3_Shutdown();ImGui_ImplWin32_Shutdown();ImGui::DestroyContext();initialized=false;gameWindow=nullptr;originalWndProc=nullptr; }
    bool isOpen(){return menuOpen;} void setOpen(bool open){menuOpen=open;}
    void render(){
        if(!initialized)return; if(GetAsyncKeyState(menuKey)&1)menuOpen=!menuOpen;
        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame(); applyTheme();
        if(menuOpen){
            ImGui::SetNextWindowSize(ImVec2(1180,720),ImGuiCond_FirstUseEver); ImGui::Begin("Eminence Client",&menuOpen,ImGuiWindowFlags_NoCollapse);
            ImGui::TextUnformatted("Eminence"); ImGui::SameLine(); ImGui::TextDisabled("Utility Client | Minecraft 1.21.11"); ImGui::SameLine(ImGui::GetWindowWidth()-310);
            if(ImGui::SmallButton(dashboard?"Compact":"Dashboard"))dashboard=!dashboard; ImGui::SameLine(); if(ImGui::SmallButton("Settings"))settingsOpen=!settingsOpen; ImGui::SameLine(); if(ImGui::SmallButton("Configs"))configsOpen=!configsOpen;
            ImGui::Separator();
            if(settingsOpen)drawSettings(); else if(configsOpen){ ImGui::BeginChild("Configs",ImVec2(0,0),true); ImGui::TextUnformatted("Configs"); ImGui::Separator(); ImGui::TextDisabled("Config file I/O is kept separate from renderer state; visual settings already live in the shared module store."); ImGui::EndChild(); }
            else if(dashboard)drawDashboard(); else drawOverlay();
            ImGui::End();
        }
        ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    LRESULT CALLBACK wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam){
        if(waitingForMenuKey && msg==WM_KEYDOWN){
            if(wParam==VK_ESCAPE){ waitingForMenuKey=false; return 0; }
            menuKey=static_cast<int>(wParam); waitingForMenuKey=false; return 0;
        }
        if(msg==WM_KEYDOWN && !menuOpen && wParam!=VK_SHIFT && wParam!=VK_CONTROL && wParam!=VK_MENU){
            for(const auto& module : ModuleRegistry::instance().snapshot()){
                if(module.key == static_cast<int>(wParam)){
                    ModuleRegistry::instance().setState(module.name, !module.enabled);
                    break;
                }
            }
        }
        if(menuOpen){
            if(waitingForKey&&msg==WM_KEYDOWN){ if(wParam==VK_ESCAPE){ModuleRegistry::instance().setKey(keyTarget,0);waitingForKey=false;keyTarget.clear();return 0;} if(wParam!=VK_SHIFT&&wParam!=VK_CONTROL&&wParam!=VK_MENU){ModuleRegistry::instance().setKey(keyTarget,(int)wParam);waitingForKey=false;keyTarget.clear();return 0;} }
            ImGui_ImplWin32_WndProcHandler(hwnd,msg,wParam,lParam);
            if(msg>=WM_MOUSEFIRST&&msg<=WM_MOUSELAST)return 0; if(msg==WM_KEYDOWN||msg==WM_KEYUP||msg==WM_CHAR||msg==WM_SYSKEYDOWN||msg==WM_SYSKEYUP)return 0;
        }
        return CallWindowProcW(originalWndProc,hwnd,msg,wParam,lParam);
    }
}
