#pragma once

#include <memory>
#include <vector>
#include <Windows.h>

#include "Unit.h"
#include "ICombatTarget.h"
#include "Observer.h"

class Base : public ICombatTarget,
             public Observer,
             public std::enable_shared_from_this<Base>
{
private:
    COORD location;
    Health health;
    Armor armor;
    std::vector<std::weak_ptr<Unit>> base_units;
    int max_units_amount;
    bool is_player;

    void remove_inactive_units();

public:
    Base(COORD location, int health, int armor, int max_units_amount, bool is_player);
    ~Base() override = default;

    std::shared_ptr<Unit> create_unit(UnitType type, COORD spawn, bool player_team);

    bool is_player_base() const { return is_player; }
    bool is_alive() const { return !health.is_zero(); }
    int get_alive_units_count() const;
    int get_max_units_amount() const { return max_units_amount; }
    const std::vector<std::weak_ptr<Unit>>& get_units() const { return base_units; }

    int get_health_value() const override { return health.get_health(); }
    int get_armor_value() const override { return armor.get_armor(); }
    COORD get_position() const override { return location; }

    bool owns_unit(const Unit& unit) const;
    void take_damage(int damage) override;
    void take_healing(int amount) override;
    void on_unit_event(Unit& unit, EventType event) override;
};
