#pragma once

#include <vector>
#include <memory>
#include <string>
#include <Windows.h>

#include "Unit.h"
#include "Base.h"
#include "Terrain.h"
#include "NeutralObject.h"

class GameField
{
private:
    int width;
    int height;
    size_t max_capacity;

    std::vector<std::shared_ptr<Unit>> units;
    std::vector<std::shared_ptr<Base>> bases;
    std::vector<std::unique_ptr<Terrain>> terrains;
    std::vector<std::unique_ptr<NeutralObject>> neutral_objects;
    std::vector<std::string> action_log;
    int last_move_distance;

    bool is_inside_field(COORD position) const;
    bool is_unit_at(COORD position, size_t ignored_index) const;
    bool is_base_at(COORD position) const;
    const Terrain* get_terrain_at(COORD position) const;
    std::shared_ptr<Base> find_owning_base(const Unit& unit) const;
    bool activate_neutral_object(NeutralObject& object, Unit& unit, Base& base);
    void update_terrain_bonus(Unit& unit);
    bool is_position_free_for_unit(COORD position, const Unit& unit, size_t ignored_index) const;

public:
    GameField(int w, int h, size_t max_cap);
    ~GameField() = default;

    GameField(const GameField&) = delete;
    GameField& operator=(const GameField&) = delete;
    GameField(GameField&&) noexcept = default;
    GameField& operator=(GameField&&) noexcept = default;

    bool add_unit(const std::shared_ptr<Unit>& unit);
    bool add_base(const std::shared_ptr<Base>& base);
    bool add_terrain(std::unique_ptr<Terrain> terrain);
    bool add_neutral_object(std::unique_ptr<NeutralObject> object);
    bool create_unit_for_base(const std::shared_ptr<Base>& base, UnitType type, COORD spawn);

    bool process_deaths();
    bool move_unit_by_index(size_t index, COORD new_pos, std::string& error_msg);
    bool find_next_step_towards(const Unit& unit, COORD target, COORD& next_step) const;

    size_t get_units_count() const { return units.size(); }
    size_t get_alive_units_count(bool player_team) const;
    int get_last_move_distance() const { return last_move_distance; }
    Unit& get_unit_by_index(size_t index) { return *units.at(index); }
    const Unit& get_unit_by_index(size_t index) const { return *units.at(index); }
    const std::vector<std::shared_ptr<Unit>>& get_all_units() const { return units; }
    const std::vector<std::shared_ptr<Base>>& get_bases() const { return bases; }
    std::shared_ptr<Base> get_base(bool player_team) const;

    void add_log(const std::string& message);
    void draw() const;
};
