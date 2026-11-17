#pragma once

#include <memory>
#include <Windows.h>

#include "NeutralObject.h"

enum class NeutralObjectType
{
    TreasureChest,
    Trap,
    PowerFountain,
    TrainingCamp
};

class NeutralObjectFactory
{
public:
    static std::unique_ptr<NeutralObject> create_object(NeutralObjectType type, COORD position);
};
