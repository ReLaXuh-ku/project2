#pragma once

#include <memory>
#include <Windows.h>
#include "Unit.h"

class UnitFactory
{
public:
    static std::shared_ptr<Unit> create_unit(
        UnitType type,
        COORD spawn_pos,
        bool is_player
    );
};
