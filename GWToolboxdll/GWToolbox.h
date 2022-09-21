#pragma once

#include <GWCA/Managers/GameThreadMgr.h>

#include <ToolboxModule.h>
#include <ToolboxUIElement.h>
#include <Utf8.h>
#include <PluginManager.h>

DWORD __stdcall SafeThreadEntry(LPVOID mod);
DWORD __stdcall ThreadEntry(LPVOID);

LRESULT CALLBACK SafeWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) noexcept;
LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

class GWToolbox {
public:
    static GWToolbox& Instance() {
        static GWToolbox instance;
        return instance;
    }

    static HMODULE GetDLLModule();
    void Draw(IDirect3DDevice9* device);
    void Update(GW::HookStatus *);

    void Initialize();
    void Terminate();

    void OpenSettingsFile();
    void LoadModuleSettings();
    void SaveSettings();
    static void FlashWindow();

    void StartSelfDestruct() {
        if (initialized) {
            SaveSettings();
            for (ToolboxModule* module : modules) {
                module->SignalTerminate();
            }
        }
        must_self_destruct = true;
    }
    bool must_self_destruct = false;    // is true when toolbox should quit

    bool RegisterModule(ToolboxModule* m) {
        if (std::find(modules.begin(), modules.end(), m) == modules.end())
            return modules.push_back(m), true;
        return false;
    }
    bool RegisterUIElement(ToolboxUIElement* e) {
        if (std::find(uielements.begin(), uielements.end(), e) == uielements.end())
            return uielements.push_back(e), true;
        return false;
    }

    const std::vector<ToolboxModule*>& GetModules() const { return modules; }
    const std::vector<ToolboxModule*>& GetCoreModules() const { return core_modules; }
    const std::vector<ToolboxUIElement*>& GetUIElements() const { return uielements; }

    bool right_mouse_down = false;

    static void OnHandleCrash(char* description, uint32_t param_2, uint32_t param_3, uint32_t param_4, uint32_t param_5, uint32_t param_6);

    bool IsInitialized() { return initialized; };

    void AddPlugin(TBModule* mod) { plugins.push_back(mod); }
    PluginManager& GetPluginManger() { return plugin_manager; };
private:
    std::vector<ToolboxModule*> modules;

    // List of modules that can't be disabled
    std::vector<ToolboxModule*> core_modules;
    // List of modules that can be disabled
    std::vector<ToolboxModule*> optional_modules;
    // List of modules that are UI elements. They can be disable
    std::vector<ToolboxUIElement*> uielements;
    // Plugins
    std::vector<TBModule*> plugins;



    GW::HookEntry Update_Entry;

    PluginManager plugin_manager;

    GW::HookEntry HandleCrash_Entry;

    bool initialized = false;

};
