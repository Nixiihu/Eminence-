#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

enum class Category { Legit, SemiRage, Visual, Misc };

struct Module {
    std::string name;
    Category category;
    std::string description;
    bool enabled = false;
    int key = 0;
};

class ModuleRegistry {
public:
    static ModuleRegistry& instance();
    void initialize();
    std::vector<Module> snapshot() const;
    bool setState(const std::string& name, bool enabled);
    bool getState(const std::string& name) const;
    bool setKey(const std::string& name, int key);
    int getKey(const std::string& name) const;
    bool setBool(const std::string& module, const std::string& setting, bool value);
    bool getBool(const std::string& module, const std::string& setting, bool fallback = false) const;
    bool setFloat(const std::string& module, const std::string& setting, float value);
    float getFloat(const std::string& module, const std::string& setting, float fallback = 0.0f) const;
    bool setInt(const std::string& module, const std::string& setting, int value);
    int getInt(const std::string& module, const std::string& setting, int fallback = 0) const;

private:
    ModuleRegistry() = default;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Module> modules_;
    std::unordered_map<std::string, bool> boolSettings_;
    std::unordered_map<std::string, float> floatSettings_;
    std::unordered_map<std::string, int> intSettings_;
    static std::string key(const std::string& module, const std::string& setting);
};
