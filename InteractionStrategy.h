#pragma once

class Unit;
class Base;

struct InteractionContext
{
    Unit& unit;
    Base& base;
};

class InteractionStrategy
{
public:
    virtual ~InteractionStrategy() = default;

    virtual bool apply(Unit& unit, Base& base) = 0;
};
