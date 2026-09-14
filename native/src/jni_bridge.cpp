#include "module.hpp"
#include "native_core.hpp"

#include <jni.h>
#include <windows.h>
#include <string>
#include <vector>

namespace {
    JNIEXPORT jboolean JNICALL setModuleState(JNIEnv*, jclass, jstring, jboolean);
    JNIEXPORT jboolean JNICALL getModuleState(JNIEnv*, jclass, jstring);
    JNIEXPORT jboolean JNICALL setBoolSetting(JNIEnv*, jclass, jstring, jstring, jboolean);
    JNIEXPORT jboolean JNICALL getBoolSetting(JNIEnv*, jclass, jstring, jstring, jboolean);
    JNIEXPORT jboolean JNICALL setFloatSetting(JNIEnv*, jclass, jstring, jstring, jfloat);
    JNIEXPORT jfloat JNICALL getFloatSetting(JNIEnv*, jclass, jstring, jstring, jfloat);
    JNIEXPORT jboolean JNICALL setIntSetting(JNIEnv*, jclass, jstring, jstring, jint);
    JNIEXPORT jint JNICALL getIntSetting(JNIEnv*, jclass, jstring, jstring, jint);
    JNIEXPORT jobjectArray JNICALL getModules(JNIEnv*, jclass);
    JNIEXPORT void JNICALL setMenuOpen(JNIEnv*, jclass, jboolean);
    JNIEXPORT jboolean JNICALL isMenuOpen(JNIEnv*, jclass);

    JavaVM* g_vm = nullptr;
    jclass g_bridgeClass = nullptr;

    bool registerBridge(JNIEnv* env) {
        jclass local = env->FindClass("com/eminence/client/NativeBridge");
        if (!local)
            return false;

        g_bridgeClass = reinterpret_cast<jclass>(env->NewGlobalRef(local));
        env->DeleteLocalRef(local);

        JNINativeMethod methods[] = {
            {
                const_cast<char*>("setModuleState"),
                const_cast<char*>("(Ljava/lang/String;Z)Z"),
                reinterpret_cast<void*>(&setModuleState)
            },
            { const_cast<char*>("getModuleState"), const_cast<char*>("(Ljava/lang/String;)Z"), reinterpret_cast<void*>(&getModuleState) },
            { const_cast<char*>("setBoolSetting"), const_cast<char*>("(Ljava/lang/String;Ljava/lang/String;Z)Z"), reinterpret_cast<void*>(&setBoolSetting) },
            { const_cast<char*>("getBoolSetting"), const_cast<char*>("(Ljava/lang/String;Ljava/lang/String;Z)Z"), reinterpret_cast<void*>(&getBoolSetting) },
            { const_cast<char*>("setFloatSetting"), const_cast<char*>("(Ljava/lang/String;Ljava/lang/String;F)Z"), reinterpret_cast<void*>(&setFloatSetting) },
            { const_cast<char*>("getFloatSetting"), const_cast<char*>("(Ljava/lang/String;Ljava/lang/String;F)F"), reinterpret_cast<void*>(&getFloatSetting) },
            { const_cast<char*>("setIntSetting"), const_cast<char*>("(Ljava/lang/String;Ljava/lang/String;I)Z"), reinterpret_cast<void*>(&setIntSetting) },
            { const_cast<char*>("getIntSetting"), const_cast<char*>("(Ljava/lang/String;Ljava/lang/String;I)I"), reinterpret_cast<void*>(&getIntSetting) },
            {
                const_cast<char*>("getModules"),
                const_cast<char*>("()[Ljava/lang/String;"),
                reinterpret_cast<void*>(&getModules)
            },
            {
                const_cast<char*>("setMenuOpen"),
                const_cast<char*>("(Z)V"),
                reinterpret_cast<void*>(&setMenuOpen)
            },
            {
                const_cast<char*>("isMenuOpen"),
                const_cast<char*>("()Z"),
                reinterpret_cast<void*>(&isMenuOpen)
            }
        };

        return env->RegisterNatives(g_bridgeClass, methods, static_cast<jint>(sizeof(methods) / sizeof(methods[0]))) == JNI_OK;
    }

    std::string toString(JNIEnv* env, jstring value) {
        if (!value) return {};
        const char* chars = env->GetStringUTFChars(value, nullptr);
        std::string result(chars ? chars : "");
        if (chars) env->ReleaseStringUTFChars(value, chars);
        return result;
    }

    JNIEXPORT jboolean JNICALL setModuleState(JNIEnv* env, jclass, jstring name, jboolean state) {
        return ModuleRegistry::instance().setState(toString(env, name), state) ? JNI_TRUE : JNI_FALSE;
    }

    JNIEXPORT jboolean JNICALL getModuleState(JNIEnv* env, jclass, jstring name) {
        return ModuleRegistry::instance().getState(toString(env, name)) ? JNI_TRUE : JNI_FALSE;
    }

    JNIEXPORT jboolean JNICALL setBoolSetting(JNIEnv* env, jclass, jstring m, jstring s, jboolean v) {
        return ModuleRegistry::instance().setBool(toString(env,m), toString(env,s), v == JNI_TRUE) ? JNI_TRUE : JNI_FALSE;
    }
    JNIEXPORT jboolean JNICALL getBoolSetting(JNIEnv* env, jclass, jstring m, jstring s, jboolean fallback) {
        return ModuleRegistry::instance().getBool(toString(env,m), toString(env,s), fallback == JNI_TRUE) ? JNI_TRUE : JNI_FALSE;
    }
    JNIEXPORT jboolean JNICALL setFloatSetting(JNIEnv* env, jclass, jstring m, jstring s, jfloat v) {
        return ModuleRegistry::instance().setFloat(toString(env,m), toString(env,s), static_cast<float>(v)) ? JNI_TRUE : JNI_FALSE;
    }
    JNIEXPORT jfloat JNICALL getFloatSetting(JNIEnv* env, jclass, jstring m, jstring s, jfloat fallback) {
        return ModuleRegistry::instance().getFloat(toString(env,m), toString(env,s), static_cast<float>(fallback));
    }
    JNIEXPORT jboolean JNICALL setIntSetting(JNIEnv* env, jclass, jstring m, jstring s, jint v) {
        return ModuleRegistry::instance().setInt(toString(env,m), toString(env,s), static_cast<int>(v)) ? JNI_TRUE : JNI_FALSE;
    }
    JNIEXPORT jint JNICALL getIntSetting(JNIEnv* env, jclass, jstring m, jstring s, jint fallback) {
        return ModuleRegistry::instance().getInt(toString(env,m), toString(env,s), static_cast<int>(fallback));
    }

    JNIEXPORT jobjectArray JNICALL getModules(JNIEnv* env, jclass) {
        auto modules = ModuleRegistry::instance().snapshot();
        jclass stringClass = env->FindClass("java/lang/String");
        jobjectArray result = env->NewObjectArray(
            static_cast<jsize>(modules.size()), stringClass, nullptr
        );

        for (jsize i = 0; i < static_cast<jsize>(modules.size()); ++i) {
            jstring value = env->NewStringUTF(modules[i].name.c_str());
            env->SetObjectArrayElement(result, i, value);
            env->DeleteLocalRef(value);
        }

        env->DeleteLocalRef(stringClass);
        return result;
    }

    JNIEXPORT void JNICALL setMenuOpen(JNIEnv*, jclass, jboolean open) {
        NativeCore::setMenuOpen(open == JNI_TRUE);
    }

    JNIEXPORT jboolean JNICALL isMenuOpen(JNIEnv*, jclass) {
        return NativeCore::menuOpen() ? JNI_TRUE : JNI_FALSE;
    }

    DWORD WINAPI registrationThread(LPVOID) {
        if (JNI_GetCreatedJavaVMs(&g_vm, 1, nullptr) != JNI_OK || !g_vm)
            return 0;

        JNIEnv* env = nullptr;
        bool attached = false;

        if (g_vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_8) != JNI_OK) {
            if (g_vm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr) != JNI_OK)
                return 0;
            attached = true;
        }

        for (int attempt = 0; attempt < 100 && !registerBridge(env); ++attempt)
            Sleep(250);

        NativeCore::initialize();

        if (attached)
            g_vm->DetachCurrentThread();

        return 0;
    }
}

extern "C" void StartJniBridge() {
    CreateThread(nullptr, 0, registrationThread, nullptr, 0, nullptr);
}
