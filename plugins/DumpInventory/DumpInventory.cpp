#include "DumpInventory.h"

#include "GWCA/GWCA.h"
#include "GWCA/Context/ItemContext.h"
#include "GWCA/GameEntities/Agent.h"
#include "GWCA/GameEntities/Item.h"
#include "GWCA/Managers/AgentMgr.h"
#include "GWCA/Managers/PlayerMgr.h"
#include "GWCA/Managers/ChatMgr.h"

import PluginUtils;

DLLAPI ToolboxPlugin* ToolboxPluginInstance()
{
    static DumpInventory instance;
    return &instance;
}

bool dumpInvCallback(const wchar_t*, int, wchar_t**);
void dumpBagToFile(std::wofstream& file, const GW::Bag* bag);

void DumpInventory::LoadSettings(const wchar_t* folder) {
    this->settingsPath = folder;
}

void DumpInventory::Initialize(ImGuiContext* ctx, ImGuiAllocFns allocator_fns, HMODULE toolbox_dll)
{
    ToolboxPlugin::Initialize(ctx, allocator_fns, toolbox_dll);
    GW::Initialize();
    GW::Chat::CreateCommand(L"dumpinv", GW::Chat::CmdCB(dumpInvCallback));
}

bool dumpInvCallback(const wchar_t*, int argc, wchar_t** argv) {
    const auto player = GW::Agents::GetCharacter();
    const auto player_name = GW::PlayerMgr::GetPlayerName(player->player_number);

    auto filename = std::wostringstream();
    int minBag, maxBag;
    if (argc == 2 && wcscmp(argv[1], L"storage")) {
        filename << "Storage - ";
        minBag = 8; // Storage-1
        maxBag = 22; // Storage-14
    }
    else {
        filename << "Inventory - ";
        minBag = 1;
        maxBag = 5;
    }

    filename << player_name << ".csv";

    const GW::Inventory* inventory = GW::GetItemContext()->inventory;

    {
        auto plugin = reinterpret_cast<DumpInventory*>(ToolboxPluginInstance());

        auto path = std::filesystem::path(plugin->settingsPath).parent_path() / "inventories" / filename.str();
        std::wofstream dumpFile(path);

        dumpFile << "name,type,model_id,mods" << std::endl;

        for (int i = minBag; i <= maxBag; i++) {
            auto bag = inventory->bags[i];
            if (bag == nullptr) {
                continue;
            }
            dumpBagToFile(dumpFile, bag);
        }
    }

    return true;
}

void dumpBagToFile(std::wofstream& file, const GW::Bag* bag) {
    for (auto it = bag->items.begin(); it != bag->items.end(); it++) {
        auto item = *it;
        if (item == nullptr) {
            continue;
        }

        PluginUtils::EncString name = PluginUtils::EncString(item->single_item_name);

        file << name.wstring() << ",";
        file << item->type << ",";
        file << item->model_id << ",";

        for (uint32_t i = 0; i < item->mod_struct_size; i++) {
            GW::ItemModifier* mod = &(item->mod_struct[i]);
            if (mod->identifier() == 9224) {
                file << (mod->mod & 0xFFFF) << ",";
            }
        }

        file << std::endl;
    }
}
