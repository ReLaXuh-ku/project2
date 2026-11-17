#pragma once

#include <Windows.h>
#include <memory>
#include <vector>

#include "attributes.h"
#include "ICombatTarget.h"
#include "Observer.h"

enum UnitType { bruiser, samurai, mage, soldier, healer, cannoneer };

class Unit : public ICombatTarget
{
protected:
    COORD position;
    Health health;
    Armor armor;
    Attack attack;
    AttackRange range;
    Speed speed;
    UnitType type;
    bool is_player;
    bool death_logged;
    bool death_notified;
    int terrain_armor_bonus;
    std::vector<std::weak_ptr<Observer>> observers;

    void notify_observers(EventType event);

public:
    Unit(COORD pos, int hp, int arm, int atk, int rng, int s, UnitType t, bool player);
    Unit(const Unit& other);
    ~Unit() override = default;

    COORD get_position() const override { return position; }
    bool is_alive() const;
    bool is_player_unit() const { return is_player; }
    bool is_death_logged() const { return death_logged; }
    void mark_death_logged() { death_logged = true; }
    UnitType get_type() const;
    virtual std::shared_ptr<Unit> clone() const = 0;

    int get_health_value() const override { return health.get_health(); }
    int get_armor_value() const override { return armor.get_armor() + terrain_armor_bonus; }
    int get_attack_value() const { return attack.get_damage(); }
    int get_range_value() const { return range.get_range(); }
    int get_speed_value() const { return speed.get_speed(); }
    void increase_attack(int amount) { attack.increase_damage(amount); }
    void increase_range(int amount) { range.increase_range(amount); }
    void set_terrain_armor_bonus(int amount) { terrain_armor_bonus = amount < 0 ? 0 : amount; }
    void remove_terrain_armor_bonus() { terrain_armor_bonus = 0; }

    void add_observer(const std::shared_ptr<Observer>& observer);
    void remove_expired_observers();
    bool can_attack(COORD target_pos) const;
    void take_damage(int damage) override;
    void take_healing(int amount) override;
    virtual void execute_action(ICombatTarget& target) = 0;
    bool can_move_to(COORD target_pos) const;
    void move_to(COORD new_pos);
};
