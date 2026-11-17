#pragma once

#include "NeutralObject.h"

class TrapStrategy : public InteractionStrategy
{
public:
    bool apply(Unit& unit, Base& base) override;
};

class Trap : public NeutralObject
{
public:
    explicit Trap(COORD position);

    bool operator-(const InteractionContext& context);

    char get_symbol() const override
    {
        return 'T';
    }
};
