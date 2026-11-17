#pragma once

#include "Terrain.h"

class TerrainProxy
{
private:
    const Terrain* terrain;

public:
    explicit TerrainProxy(const Terrain* terrain) : terrain(terrain) {}

    bool is_passable(const Unit& unit) const
    {
        return terrain == nullptr || terrain->is_passable(unit);
    }

    int get_speed_modifier(const Unit& unit) const
    {
        return terrain == nullptr ? 0 : terrain->get_speed_modifier(unit);
    }

    int get_defense_bonus(const Unit& unit) const
    {
        return terrain == nullptr ? 0 : terrain->get_defense_bonus(unit);
    }

    COORD get_position() const
    {
        return terrain == nullptr ? COORD{ 0, 0 } : terrain->get_position();
    }

    char get_symbol() const
    {
        return terrain == nullptr ? '*' : terrain->get_symbol();
    }
};
