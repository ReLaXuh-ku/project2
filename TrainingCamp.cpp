#include "TrainingCamp.h"

#include <memory>
#include "Unit.h"
#include "Base.h"

bool TrainingCampStrategy::apply(Unit& unit, Base&)
{
    if (!unit.is_alive()) return false;
    unit.increase_range(1);
    return true;
}

TrainingCamp::TrainingCamp(COORD position)
    : NeutralObject(position, std::make_unique<TrainingCampStrategy>())
{
}

bool TrainingCamp::operator/(const InteractionContext& context)
{
    return interact(context.unit, context.base);
}
