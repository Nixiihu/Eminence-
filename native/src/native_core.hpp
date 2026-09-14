#pragma once

#include <windows.h>

namespace NativeCore {
    bool initialize();
    void shutdown();
    void render();
    bool menuOpen();
    void setMenuOpen(bool open);
}
