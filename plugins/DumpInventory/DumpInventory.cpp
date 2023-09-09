#include "DumpInventory.h"

#include <mutex>
#include <regex>

#include "GWCA/GWCA.h"
#include "GWCA/Context/ItemContext.h"
#include "GWCA/GameEntities/Agent.h"
#include "GWCA/GameEntities/Item.h"
#include "GWCA/Managers/AgentMgr.h"
#include "GWCA/Managers/PlayerMgr.h"
#include "GWCA/Managers/ChatMgr.h"
#include "GWCA/Managers/UIMgr.h"
#include "GWCA/Utilities/Hooker.h"

bool dumpInvCallback(const wchar_t*, int, wchar_t**);
void dumpBagToFile(std::shared_ptr<std::wofstream> file, const GW::Bag* bag);

DLLAPI ToolboxPlugin* ToolboxPluginInstance()
{
    static DumpInventory instance;
    return &instance;
}

void DumpInventory::LoadSettings(const wchar_t* folder) {
    this->settingsPath = folder;
}

void DumpInventory::Initialize(ImGuiContext* ctx, ImGuiAllocFns allocator_fns, HMODULE toolbox_dll)
{
    ToolboxPlugin::Initialize(ctx, allocator_fns, toolbox_dll);
    GW::Initialize();
    GW::Chat::CreateCommand(L"dumpinv", GW::Chat::CmdCB(dumpInvCallback));
}

bool DumpInventory::CanTerminate()
{
    return GW::HookBase::GetInHookCount() == 0;
}

void DumpInventory::SignalTerminate()
{
    ToolboxPlugin::SignalTerminate();
    GW::Chat::DeleteCommand(L"dumpinv");
    GW::DisableHooks();
}

void DumpInventory::Terminate()
{
    ToolboxPlugin::Terminate();
    GW::Terminate();
}

bool dumpInvCallback(const wchar_t*, int argc, wchar_t** argv) {
    const auto player = GW::Agents::GetCharacter();
    const auto player_name = GW::PlayerMgr::GetPlayerName(player->player_number);

    auto filename = std::wostringstream();
    int minBag, maxBag;
    if (argc == 2 && wcscmp(argv[1], L"storage") == 0) {
        filename << "Storage - ";
        minBag = 8; // Storage-1
        maxBag = 21; // Storage-14
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
        auto dumpFile = std::make_shared<std::wofstream>(path);

        dumpFile->fill('0');
        *dumpFile << "rarity,model_id,type,name,requirement,attribute,damage/armor/energy,inscribable,prefix,suffix,inscription,description" << std::endl;

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

struct ItemInfo {
    std::wstring name;
    std::wstring description;

    int model_id = -1;
    std::wstring rarity;
    GW::Constants::ItemType type = GW::Constants::ItemType::Salvage;

    int minDamage = -1;
    int maxDamage = -1;
    int armor = -1;
    int energy = -1;

    int requirement = -1;
    int8_t attribute = -1;

    bool inscribable = false;
    int prefix = -1;
    int suffix = -1;
    int inscription = -1;
};

constexpr std::wstring_view attributeNames[] = {
    L"Fast Casting", L"Illusion Magic", L"Domination Magic", L"Inspiration Magic",
    L"Blood Magic", L"Death Magic", L"Soul Reaping", L"Curses",
    L"Air Magic", L"Earth Magic", L"Fire Magic", L"Water Magic", L"Energy Storage",
    L"Healing Prayers", L"Smiting Prayers", L"Protection Prayers", L"Divine Favor",
    L"Strength", L"Axe Mastery", L"Hammer Mastery", L"Swordsmanship", L"Tactics",
    L"Beast Mastery", L"Expertise", L"Wilderness Survival", L"Marksmanship",
    L"Attribute26", L"Attribute27", L"Attribute28",
    L"Dagger Mastery", L"Deadly Arts", L"Shadow Arts",
    L"Communing", L"Restoration Magic", L"Channeling Magic",
    L"Critical Strikes", L"Spawning Power",
    L"Spear Mastery", L"Command", L"Motivation", L"Leadership",
    L"Scythe Mastery", L"Wind Prayers", L"Earth Prayers", L"Mysticism"
};

constexpr std::wstring_view invalidAttribute = L"Invalid Attribute";

const std::wstring_view& getAttributeName(uint8_t attribute) {
    if (attribute > 44) {
        return invalidAttribute;
    }

    return attributeNames[attribute];
}

constexpr std::wstring_view speciesModNames[] = {
    L"Deathbane", L"Charrslaying", L"Trollslaying", L"Pruning", L"Skeletonslaying",
    L"Giantslaying", L"Dwarfslaying", L"Tenguslaying", L"Demonslaying", L"Dragonslaying",
    L"Ogreslaying",
};

constexpr std::wstring_view invalidSpeciesMod = L"INVALID SLAYING MOD";

const std::wstring_view& getSpeciesModName(uint32_t species) {
    if (species > 10) {
        return invalidSpeciesMod;
    }
    return speciesModNames[species];
}

bool isPrefix(int upgradeId) {
    return (upgradeId >= 129 && upgradeId <= 174)
        || (upgradeId >= 302 && upgradeId <= 314)
        || (upgradeId >= 327 && upgradeId <= 329)
        || (upgradeId >= 363 && upgradeId <= 385)
        || (upgradeId >= 523 && upgradeId <= 528);
}

bool isSuffix(int upgradeId) {
    return (upgradeId >= 195 && upgradeId <= 235)
        || (upgradeId >= 321 && upgradeId <= 326)
        || (upgradeId >= 337 && upgradeId <= 342)
        || (upgradeId >= 351 && upgradeId <= 354)
        || (upgradeId >= 392 && upgradeId <= 403)
        || (upgradeId >= 535 && upgradeId <= 540);
}

bool isInscription(int upgradeId) {
    return (upgradeId >= 348 && upgradeId <= 350)
        || (upgradeId >= 355 && upgradeId <= 362)
        || (upgradeId >= 438 && upgradeId <= 477)
        || upgradeId == 542 || upgradeId == 543;
}

struct CallbackParam {
    ItemInfo* info;
    const wchar_t* encodedDescription;
    std::shared_ptr<std::wofstream> file;

    CallbackParam(ItemInfo* info, const wchar_t* desc, std::shared_ptr<std::wofstream> file)
        : info(info), encodedDescription(desc), file(file) {
    }

    ~CallbackParam() {
        file.reset();

        if (info != nullptr) {
            delete info;
        }
    }
};

void stripTags(std::wstring& decodedString) {
    if (decodedString.empty()) {
        return;
    }

    static const std::wregex tag(L"<[^>]+>");
    decodedString = std::regex_replace(decodedString, tag, L"");
}

std::mutex dumpBagToFile_mutex;

void dumpBagToFile_callback2(void* _param, wchar_t* translated) {
    auto param = reinterpret_cast<CallbackParam*>(_param);

    std::wstring translatedString(translated);
    stripTags(translatedString);
    
    std::wregex endl(L"\r?\n");
    translatedString = std::regex_replace(translatedString, endl, L"; ");

    std::wregex comma(L",");
    translatedString = std::regex_replace(translatedString, comma, L"\\,");

    param->info->description = translatedString;

    auto info = param->info;
    auto file = param->file;
    std::unique_lock<std::mutex> lock(dumpBagToFile_mutex);

    *file << info->rarity << ",";
    *file << info->model_id << ",";
    *file << static_cast<int>(info->type) << ",";
    *file << info->name << ",";
    *file << info->requirement << ",";
    *file << getAttributeName(info->attribute) << ",";

    if (info->minDamage != -1) {
        *file << "Damage: " << info->minDamage << "-" << info->maxDamage;
    }
    else if (info->energy != -1) {
        *file << "Energy: +" << info->energy;
    }
    else if (info->armor != -1) {
        *file << "Armor: " << info->armor;
    }
    *file << ",";

    *file << (info->inscribable ? "Inscribable" : "OS") << ",";
    *file << info->prefix << ",";
    *file << info->suffix << ",";
    *file << info->inscription << ",";

    *file << info->description << ",";
    *file << std::endl;

    delete param;
};

void dumpBagToFile_callback1(void* _param, wchar_t* translated) {
    auto param = reinterpret_cast<CallbackParam*>(_param);
    std::wstring translatedString(translated);
    std::wregex expr(L"<c=@Item(Common|Enhance|Uncommon|Rare|Unique)>");
    std::wsmatch match;
    
    if (std::regex_search(translatedString, match, expr)) {
        std::wstring rarity = match[1];

        if (rarity.compare(L"Common") == 0) {
            param->info->rarity = std::wstring(L"White");
        }
        else if (rarity.compare(L"Enhance") == 0) {
            param->info->rarity = std::wstring(L"Blue");
        }
        else if (rarity.compare(L"Uncommon") == 0) {
            param->info->rarity = std::wstring(L"Purple");
        }
        else if (rarity.compare(L"Rare") == 0) {
            param->info->rarity = std::wstring(L"Gold");
        }
        else if (rarity.compare(L"Unique") == 0) {
            param->info->rarity = std::wstring(L"Green");
        }
        else {
            // Shouldn't happen
            // TODO: log error to chat
        }
    }

    stripTags(translatedString);
    param->info->name = translatedString;

    GW::UI::AsyncDecodeStr(param->encodedDescription, dumpBagToFile_callback2, _param);
}

void dumpBagToFile(std::shared_ptr<std::wofstream> file, const GW::Bag* bag) {
    for (auto it = bag->items.begin(); it != bag->items.end(); it++) {
        auto item = *it;
        if (item == nullptr) {
            continue;
        }

        ItemInfo* info = new ItemInfo();
        info->model_id = item->model_id;
        info->type = static_cast<GW::Constants::ItemType>(item->type);
        info->inscribable = (item->interaction & 0x08000000) != 0;

        for (uint32_t i = 0; i < item->mod_struct_size; i++) {
            auto mod = item->mod_struct[i];

            switch (mod.identifier()) {
            case 0x8080:
                //info->bane_species = getSpeciesModName(mod.arg1());
                break;
            case 0x2408:
                {
                    int upgradeId = static_cast<int>(mod.mod & 0xFFFF);

                    if (isPrefix(upgradeId)) {
                        info->prefix = upgradeId;
                    }
                    else if (isSuffix(upgradeId)) {
                        info->suffix = upgradeId;
                    }
                    else if (isInscription(upgradeId)) {
                        info->inscription = upgradeId;
                    }
                    else {
                        // Shouldn't happen
                        // TODO: output to chat
                    }
                }
                break;
            case 0x2798:
                info->attribute = static_cast<int8_t>(mod.arg1());
                info->requirement = mod.arg2();
                break;
            case 0xA7A8:
                info->minDamage = mod.arg2();
                info->maxDamage = mod.arg1();
                break;
            case 0xA7B8:
            case 0xA3C8:
                info->armor = mod.arg1();
                break;
            case 0x67C8:
                info->energy = mod.arg1();
                break;
            }
        }

        CallbackParam* param = new CallbackParam(info, item->info_string, file);

        GW::UI::AsyncDecodeStr(item->single_item_name, dumpBagToFile_callback1, reinterpret_cast<void*>(param));
    }
}
