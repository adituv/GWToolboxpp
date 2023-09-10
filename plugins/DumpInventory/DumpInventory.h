#pragma once

#include "ItemUpgrade.h"

#include <ToolboxPlugin.h>

class DumpInventory : public ToolboxPlugin {
public:
    DumpInventory() = default;
    ~DumpInventory() override = default;

    const char* Name() const override { return "Dump Inventory"; }
    bool HasSettings() const override { return true; }
    void LoadSettings(const wchar_t*) override;
    void Initialize(ImGuiContext* ctx, ImGuiAllocFns allocator_fns, HMODULE toolbox_dll) override;
    bool CanTerminate() override;
    void SignalTerminate() override;
    void Terminate() override;

    std::wstring settingsPath;
};
