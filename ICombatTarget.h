#pragma once
#include <Windows.h>

class ICombatTarget
{
public:
    virtual ~ICombatTarget() = default;

    virtual void take_damage(int damage) = 0;
    virtual void take_healing(int amount) = 0;

    virtual int get_armor_value() const = 0;
    virtual int get_health_value() const = 0;
    virtual COORD get_position() const = 0;
};
