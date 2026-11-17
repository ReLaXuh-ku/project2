#pragma once

#include <memory>
#include "Unit.h"

class Bruiser : public Unit
{
public:
    Bruiser(COORD pos, bool player) : Unit(pos, 800, 30, 50, 1, 1, UnitType::bruiser, player) {}
    std::shared_ptr<Unit> clone() const override { return std::make_shared<Bruiser>(*this); }
    void execute_action(ICombatTarget& target) override
    {
        if (can_attack(target.get_position())) target.take_damage(attack.calculate_damage(target.get_armor_value()));
    }
};

class Samurai : public Unit
{
public:
    Samurai(COORD pos, bool player) : Unit(pos, 650, 20, 60, 1, 2, UnitType::samurai, player) {}
    std::shared_ptr<Unit> clone() const override { return std::make_shared<Samurai>(*this); }
    void execute_action(ICombatTarget& target) override
    {
        if (can_attack(target.get_position())) target.take_damage(attack.calculate_damage(target.get_armor_value()));
    }
};
