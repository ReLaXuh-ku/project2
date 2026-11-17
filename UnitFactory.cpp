#include "UnitFactory.h"

#include "MeleeUnits.h"
#include "RangedUnits.h"
#include "GlobalUnits.h"

std::shared_ptr<Unit> UnitFactory::create_unit(
    UnitType type,
    COORD spawn_pos,
    bool is_player
)
{
    switch (type)
    {
    case UnitType::bruiser:
        return std::make_shared<Bruiser>(spawn_pos, is_player);

    case UnitType::samurai:
        return std::make_shared<Samurai>(spawn_pos, is_player);

    case UnitType::soldier:
        return std::make_shared<Soldier>(spawn_pos, is_player);

    case UnitType::mage:
        return std::make_shared<Mage>(spawn_pos, is_player);

    case UnitType::healer:
        return std::make_shared<Healer>(spawn_pos, is_player);

    case UnitType::cannoneer:
        return std::make_shared<Cannoneer>(spawn_pos, is_player);

    default:
        return nullptr;
    }
}
