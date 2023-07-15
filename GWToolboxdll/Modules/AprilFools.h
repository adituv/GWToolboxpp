#pragma once

#include <ToolboxModule.h>

class AprilFools : public ToolboxModule {
    AprilFools() = default;
    ~AprilFools() override = default;

public:
    static AprilFools& Instance()
    {
        static AprilFools instance;
        return instance;
    }

    const char* Name() const override { return "April Fools"; }
    bool HasSettings() override { return false; }

    void Initialize() override;
    void Terminate() override;
    void Update(float delta) override;
    void SetInfected(GW::Agent* agent, bool is_infected);
    void SetEnabled(bool is_enabled);
};
