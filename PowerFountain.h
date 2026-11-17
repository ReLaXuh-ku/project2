#pragma once

#include "NeutralObject.h"

class PowerFountainStrategy : public InteractionStrategy
{
public:
    bool apply(Unit& unit, Base& base) override;
};

class PowerFountain : public NeutralObject
{
public:
    explicit PowerFountain(COORD position);

    bool operator*(const InteractionContext& context);

    char get_symbol() const override
    {
        return 'F';
    }
};
