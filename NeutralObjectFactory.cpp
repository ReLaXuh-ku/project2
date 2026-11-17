#include "NeutralObjectFactory.h"

#include "TreasureChest.h"
#include "Trap.h"
#include "PowerFountain.h"
#include "TrainingCamp.h"

std::unique_ptr<NeutralObject> NeutralObjectFactory::create_object(NeutralObjectType type, COORD position)
{
    switch (type)
    {
    case NeutralObjectType::TreasureChest: return std::make_unique<TreasureChest>(position);
    case NeutralObjectType::Trap: return std::make_unique<Trap>(position);
    case NeutralObjectType::PowerFountain: return std::make_unique<PowerFountain>(position);
    case NeutralObjectType::TrainingCamp: return std::make_unique<TrainingCamp>(position);
    default: return nullptr;
    }
}
