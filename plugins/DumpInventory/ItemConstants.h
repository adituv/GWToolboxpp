#pragma once

#include <cstdint>
#include <string>

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

enum class UpgradeType {
    Unknown, Prefix, Suffix, Inscription
};

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
