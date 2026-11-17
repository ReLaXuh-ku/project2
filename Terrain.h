#pragma once

#include <Windows.h>
#include "Unit.h"

class Terrain
{
protected:
    COORD position;

public:
    explicit Terrain(COORD position)
        : position(position)
    {
    }

    virtual ~Terrain() = default;

    COORD get_position() const
    {
        return position;
    }

    virtual bool is_passable(const Unit& unit) const = 0;

    virtual int get_speed_modifier(const Unit& unit) const = 0;

    virtual int get_defense_bonus(const Unit& unit) const = 0;

    virtual char get_symbol() const = 0;
};

class Mountain : public Terrain
{
public:
    explicit Mountain(COORD position)
        : Terrain(position)
    {
    }

    bool is_passable(const Unit& unit) const override
    {
        return unit.get_type() == UnitType::samurai;
    }

    int get_speed_modifier(const Unit& unit) const override
    {
        return 0;
    }

    int get_defense_bonus(const Unit& unit) const override
    {
        return 0;
    }

    char get_symbol() const override
    {
        return '^';
    }
};

class Swamp : public Terrain
{
public:
    explicit Swamp(COORD position)
        : Terrain(position)
    {
    }

    bool is_passable(const Unit& unit) const override
    {
        return true;
    }

    int get_speed_modifier(const Unit& unit) const override
    {
        return unit.get_speed_value() == 1 ? -1 : 0;
    }

    int get_defense_bonus(const Unit& unit) const override
    {
        return 0;
    }

    char get_symbol() const override
    {
        return '~';
    }
};

class Forest : public Terrain
{
public:
    explicit Forest(COORD position)
        : Terrain(position)
    {
    }

    bool is_passable(const Unit& unit) const override
    {
        return true;
    }

    int get_speed_modifier(const Unit& unit) const override
    {
        return 0;
    }

    int get_defense_bonus(const Unit& unit) const override
    {
        return 10;
    }

    char get_symbol() const override
    {
        return '#';
    }
};
