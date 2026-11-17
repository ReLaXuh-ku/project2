#pragma once

#include "NeutralObject.h"

class TreasureChestStrategy : public InteractionStrategy
{
public:
    bool apply(Unit& unit, Base& base) override;
};

class TreasureChest : public NeutralObject
{
public:
    explicit TreasureChest(COORD position);

    bool operator+(const InteractionContext& context);

    char get_symbol() const override
    {
        return 'M';
    }
};
