#pragma once

#include "NeutralObject.h"

class TrainingCampStrategy : public InteractionStrategy
{
public:
    bool apply(Unit& unit, Base& base) override;
};

class TrainingCamp : public NeutralObject
{
public:
    explicit TrainingCamp(COORD position);

    bool operator/(const InteractionContext& context);

    char get_symbol() const override
    {
        return 'C';
    }
};
