#include "TreasureChest.h"

#include <memory>
#include "Unit.h"
#include "Base.h"

bool TreasureChestStrategy::apply(Unit& unit, Base& base)
{
    if (!unit.is_alive() || !base.owns_unit(unit)) return false;
    base.take_healing(200);
    return true;
}

TreasureChest::TreasureChest(COORD position)
    : NeutralObject(position, std::make_unique<TreasureChestStrategy>())
{
}

bool TreasureChest::operator+(const InteractionContext& context)
{
    return interact(context.unit, context.base);
}
