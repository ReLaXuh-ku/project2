#include "PowerFountain.h"

#include <memory>
#include "Unit.h"
#include "Base.h"

bool PowerFountainStrategy::apply(Unit& unit, Base&)
{
    if (!unit.is_alive()) return false;
    unit.increase_attack(50);
    return true;
}

PowerFountain::PowerFountain(COORD position)
    : NeutralObject(position, std::make_unique<PowerFountainStrategy>())
{
}

bool PowerFountain::operator*(const InteractionContext& context)
{
    return interact(context.unit, context.base);
}
