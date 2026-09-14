#include "native_core.hpp"
#include "module.hpp"
#include "imgui_menu.hpp"

namespace NativeCore {
    bool initialize() {
        ModuleRegistry::instance().initialize();
        return true;
    }

    void shutdown() {
        ImGuiMenu::shutdown();
    }

    void render() {
        // Menu key handling lives inside ImGuiMenu so the user-configured key is authoritative.
        ImGuiMenu::render();
    }

    bool menuOpen() {
        return ImGuiMenu::isOpen();
    }

    void setMenuOpen(bool open) {
        ImGuiMenu::setOpen(open);
    }
}
