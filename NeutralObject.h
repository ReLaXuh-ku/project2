#pragma once

#include <memory>
#include <Windows.h>

#include "InteractionStrategy.h"

class Unit;
class Base;

class NeutralObject
{
protected:
    COORD position;
    std::unique_ptr<InteractionStrategy> strategy;
    bool active;

    bool apply_strategy(Unit& unit, Base& base);

public:
    NeutralObject(COORD position, std::unique_ptr<InteractionStrategy> strategy);
    virtual ~NeutralObject() = default;

    NeutralObject(const NeutralObject&) = delete;
    NeutralObject& operator=(const NeutralObject&) = delete;

    COORD get_position() const { return position; }
    bool is_active() const { return active; }
    virtual char get_symbol() const = 0;
    bool interact(Unit& unit, Base& base);
};
