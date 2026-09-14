#include "native_core.hpp"
#include "imgui_menu.hpp"
#include "jni_bridge.hpp"

#include <windows.h>
#include <gl/GL.h>
#include <MinHook.h>

namespace {
    using WglSwapBuffersFn = BOOL(WINAPI*)(HDC);
    WglSwapBuffersFn originalSwapBuffers = nullptr;
    bool hooked = false;

    BOOL WINAPI hookSwapBuffers(HDC hdc) {
        HWND hwnd = WindowFromDC(hdc);

        if (hwnd && !ImGuiMenu::isOpen()) {
            // Initialization is idempotent. It occurs on a thread with a
            // current OpenGL context, immediately before drawing.
            static bool initialized = false;
            if (!initialized) {
                initialized = ImGuiMenu::initialize(hwnd);
            }
        } else if (hwnd) {
            // If the menu was already opened, initialization has already
            // happened. render() remains responsible for frame lifecycle.
        }

        if (hwnd)
            NativeCore::render();

        return originalSwapBuffers(hdc);
    }

    DWORD WINAPI initThread(LPVOID) {
        StartJniBridge();

        if (MH_Initialize() != MH_OK)
            return 0;

        HMODULE opengl = GetModuleHandleW(L"opengl32.dll");
        if (!opengl)
            opengl = LoadLibraryW(L"opengl32.dll");

        if (!opengl)
            return 0;

        auto target = reinterpret_cast<LPVOID>(
            GetProcAddress(opengl, "wglSwapBuffers")
        );

        if (!target)
            return 0;

        if (MH_CreateHook(
                target,
                reinterpret_cast<LPVOID>(&hookSwapBuffers),
                reinterpret_cast<LPVOID*>(&originalSwapBuffers)
            ) != MH_OK)
            return 0;

        if (MH_EnableHook(target) != MH_OK)
            return 0;

        hooked = true;
        return 0;
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        CreateThread(nullptr, 0, initThread, nullptr, 0, nullptr);
    } else if (reason == DLL_PROCESS_DETACH) {
        if (hooked) {
            MH_DisableHook(MH_ALL_HOOKS);
            MH_Uninitialize();
        }
        NativeCore::shutdown();
    }

    return TRUE;
}
