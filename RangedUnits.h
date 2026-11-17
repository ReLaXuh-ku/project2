#pragma once

#include <memory>
#include "Unit.h"

class Soldier : public Unit
{
public:
    Soldier(COORD pos, bool player) : Unit(pos, 480, 15, 60, 3, 2, UnitType::soldier, player) {}
    std::shared_ptr<Unit> clone() const override { return std::make_shared<Soldier>(*this); }
    void execute_action(ICombatTarget& target) override
    {
        if (can_attack(target.get_position())) target.take_damage(attack.calculate_damage(target.get_armor_value()));
    }
};

class Mage : public Unit
{
public:
    Mage(COORD pos, bool player) : Unit(pos, 400, 10, 80, 4, 1, UnitType::mage, player) {}
    std::shared_ptr<Unit> clone() const override { return std::make_shared<Mage>(*this); }
    void execute_action(ICombatTarget& target) override
    {
        if (can_attack(target.get_position())) target.take_damage(attack.calculate_damage(target.get_armor_value()));
    }
};
