#include "NeutralObject.h"

#include <utility>

NeutralObject::NeutralObject(COORD position, std::unique_ptr<InteractionStrategy> strategy)
    : position(position), strategy(std::move(strategy)), active(true)
{
}

bool NeutralObject::apply_strategy(Unit& unit, Base& base)
{
    if (!active || strategy == nullptr || !strategy->apply(unit, base))
    {
        return false;
    }

    active = false;
    return true;
}

bool NeutralObject::interact(Unit& unit, Base& base)
{
    return apply_strategy(unit, base);
}
