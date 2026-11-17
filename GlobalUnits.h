#pragma once

#include <memory>
#include "Unit.h"

class Healer : public Unit
{
public:
    Healer(COORD pos, bool player) : Unit(pos, 250, 5, 30, 999, 1, UnitType::healer, player) {}
    std::shared_ptr<Unit> clone() const override { return std::make_shared<Healer>(*this); }
    void execute_action(ICombatTarget& target) override
    {
        if (can_attack(target.get_position())) target.take_healing(attack.get_damage());
    }
};

class Cannoneer : public Unit
{
public:
    Cannoneer(COORD pos, bool player) : Unit(pos, 250, 0, 40, 999, 1, UnitType::cannoneer, player) {}
    std::shared_ptr<Unit> clone() const override { return std::make_shared<Cannoneer>(*this); }
    void execute_action(ICombatTarget& target) override
    {
        if (can_attack(target.get_position())) target.take_damage(attack.calculate_damage(target.get_armor_value()));
    }
};
