#pragma once
#include <windows.h>

namespace ImGuiMenu {
    bool initialize(HWND hwnd);
    void shutdown();
    void render();
    bool isOpen();
    void setOpen(bool open);
    LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
}
