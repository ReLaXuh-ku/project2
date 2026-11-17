#include "Trap.h"

#include <memory>
#include "Unit.h"
#include "Base.h"

bool TrapStrategy::apply(Unit& unit, Base&)
{
    if (!unit.is_alive()) return false;
    unit.take_damage(150);
    return true;
}

Trap::Trap(COORD position)
    : NeutralObject(position, std::make_unique<TrapStrategy>())
{
}

bool Trap::operator-(const InteractionContext& context)
{
    return interact(context.unit, context.base);
}
