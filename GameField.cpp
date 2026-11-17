#include "GameField.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <queue>
#include <limits>

#include "TreasureChest.h"
#include "Trap.h"
#include "PowerFountain.h"
#include "TrainingCamp.h"
#include "UnitFactory.h"

namespace
{
    std::string get_type_name(UnitType type)
    {
        switch (type)
        {
        case UnitType::bruiser: return "Брузер";
        case UnitType::samurai: return "Самурай";
        case UnitType::soldier: return "Солдат";
        case UnitType::mage: return "Маг";
        case UnitType::healer: return "Хилер";
        case UnitType::cannoneer: return "Канонир";
        default: return "Неизвестный";
        }
    }

    int cell_index(int x, int y, int width)
    {
        return y * width + x;
    }
}

GameField::GameField(int w, int h, size_t max_cap)
    : width(w < 3 ? 3 : w),
    height(h < 3 ? 3 : h),
    max_capacity(max_cap),
    last_move_distance(0)
{
}

bool GameField::is_inside_field(COORD position) const
{
    return position.X >= 0 && position.X < width && position.Y >= 0 && position.Y < height;
}

bool GameField::is_unit_at(COORD position, size_t ignored_index) const
{
    for (size_t i = 0; i < units.size(); ++i)
    {
        if (i == ignored_index || units[i] == nullptr || !units[i]->is_alive()) continue;
        COORD current = units[i]->get_position();
        if (current.X == position.X && current.Y == position.Y) return true;
    }
    return false;
}

bool GameField::is_base_at(COORD position) const
{
    for (const auto& base : bases)
    {
        if (base == nullptr || !base->is_alive()) continue;
        COORD current = base->get_position();
        if (current.X == position.X && current.Y == position.Y) return true;
    }
    return false;
}

const Terrain* GameField::get_terrain_at(COORD position) const
{
    for (const auto& terrain : terrains)
    {
        if (terrain != nullptr)
        {
            COORD current = terrain->get_position();
            if (current.X == position.X && current.Y == position.Y) return terrain.get();
        }
    }
    return nullptr;
}

std::shared_ptr<Base> GameField::find_owning_base(const Unit& unit) const
{
    for (const auto& base : bases)
    {
        if (base != nullptr && base->owns_unit(unit)) return base;
    }
    return nullptr;
}

bool GameField::activate_neutral_object(NeutralObject& object, Unit& unit, Base& base)
{
    if (!object.is_active()) return false;
    InteractionContext context{ unit, base };

    if (auto* chest = dynamic_cast<TreasureChest*>(&object)) return *chest + context;
    if (auto* trap = dynamic_cast<Trap*>(&object)) return *trap - context;
    if (auto* fountain = dynamic_cast<PowerFountain*>(&object)) return *fountain * context;
    if (auto* camp = dynamic_cast<TrainingCamp*>(&object)) return *camp / context;
    return false;
}

void GameField::update_terrain_bonus(Unit& unit)
{
    unit.remove_terrain_armor_bonus();
    const Terrain* terrain = get_terrain_at(unit.get_position());
    if (terrain != nullptr) unit.set_terrain_armor_bonus(terrain->get_defense_bonus(unit));
}

bool GameField::is_position_free_for_unit(COORD position, const Unit& unit, size_t ignored_index) const
{
    if (!is_inside_field(position) || is_unit_at(position, ignored_index) || is_base_at(position)) return false;
    const Terrain* terrain = get_terrain_at(position);
    return terrain == nullptr || terrain->is_passable(unit);
}

bool GameField::add_unit(const std::shared_ptr<Unit>& unit)
{
    if (unit == nullptr || !unit->is_alive() || !is_inside_field(unit->get_position())) return false;
    if (get_alive_units_count(true) + get_alive_units_count(false) >= max_capacity) return false;
    if (is_unit_at(unit->get_position(), units.size()) || is_base_at(unit->get_position())) return false;
    const Terrain* terrain = get_terrain_at(unit->get_position());
    if (terrain != nullptr && !terrain->is_passable(*unit)) return false;
    units.push_back(unit);
    update_terrain_bonus(*unit);
    return true;
}

bool GameField::add_base(const std::shared_ptr<Base>& base)
{
    if (base == nullptr || !base->is_alive() || !is_inside_field(base->get_position())) return false;
    if (is_base_at(base->get_position()) || is_unit_at(base->get_position(), units.size())) return false;
    for (const auto& current : bases)
    {
        if (current != nullptr && current->is_player_base() == base->is_player_base()) return false;
    }
    bases.push_back(base);
    return true;
}

bool GameField::add_terrain(std::unique_ptr<Terrain> terrain)
{
    if (terrain == nullptr || !is_inside_field(terrain->get_position())) return false;
    COORD position = terrain->get_position();
    if (get_terrain_at(position) != nullptr) return false;
    terrains.push_back(std::move(terrain));
    for (const auto& unit : units)
    {
        if (unit != nullptr && unit->is_alive() && unit->get_position().X == position.X && unit->get_position().Y == position.Y)
            update_terrain_bonus(*unit);
    }
    return true;
}

bool GameField::add_neutral_object(std::unique_ptr<NeutralObject> object)
{
    if (object == nullptr || !object->is_active() || !is_inside_field(object->get_position())) return false;
    COORD position = object->get_position();
    for (const auto& current : neutral_objects)
    {
        if (current != nullptr && current->is_active() && current->get_position().X == position.X && current->get_position().Y == position.Y)
            return false;
    }
    neutral_objects.push_back(std::move(object));
    return true;
}

bool GameField::create_unit_for_base(const std::shared_ptr<Base>& base, UnitType type, COORD spawn)
{
    if (base == nullptr || !base->is_alive() || !is_inside_field(spawn)) return false;
    bool belongs_to_field = false;
    for (const auto& current : bases) if (current == base) belongs_to_field = true;
    if (!belongs_to_field || base->get_alive_units_count() >= base->get_max_units_amount()) return false;
    if (get_alive_units_count(true) + get_alive_units_count(false) >= max_capacity) return false;

    std::shared_ptr<Unit> preview = UnitFactory::create_unit(type, spawn, base->is_player_base());
    if (preview == nullptr || !is_position_free_for_unit(spawn, *preview, units.size())) return false;

    std::shared_ptr<Unit> new_unit = base->create_unit(type, spawn, base->is_player_base());
    if (new_unit == nullptr) return false;
    if (!add_unit(new_unit)) return false;
    return true;
}

std::shared_ptr<Base> GameField::get_base(bool player_team) const
{
    for (const auto& base : bases)
        if (base != nullptr && base->is_player_base() == player_team) return base;
    return nullptr;
}

size_t GameField::get_alive_units_count(bool player_team) const
{
    size_t count = 0;
    for (const auto& unit : units)
        if (unit != nullptr && unit->is_alive() && unit->is_player_unit() == player_team) ++count;
    return count;
}

bool GameField::process_deaths()
{
    bool found_death = false;
    for (size_t i = 0; i < units.size(); ++i)
    {
        const auto& unit = units[i];
        if (unit != nullptr && !unit->is_alive() && !unit->is_death_logged())
        {
            add_log("Юнит [" + std::to_string(i + 1) + "] (" +
                (unit->is_player_unit() ? "Игрок" : "Бот") + ") " +
                get_type_name(unit->get_type()) + " погиб!");
            unit->mark_death_logged();
            found_death = true;
        }
    }
    return found_death;
}

bool GameField::move_unit_by_index(size_t index, COORD new_pos, std::string& error_msg)
{
    last_move_distance = 0;
    if (index >= units.size() || units[index] == nullptr || !units[index]->is_alive())
    {
        error_msg = "Выбранный юнит не существует или погиб!";
        return false;
    }

    Unit& unit = *units[index];
    COORD start = unit.get_position();
    if (!is_inside_field(new_pos))
    {
        error_msg = "Целевая клетка находится за пределами поля!";
        return false;
    }
    if (!unit.can_move_to(new_pos))
    {
        error_msg = "Недопустимое направление или расстояние движения!";
        return false;
    }

    int dx = new_pos.X - start.X;
    int dy = new_pos.Y - start.Y;
    int dir_x = (dx > 0) - (dx < 0);
    int dir_y = (dy > 0) - (dy < 0);
    int requested = std::abs(dx) + std::abs(dy);

    int effective_speed = unit.get_speed_value();
    const Terrain* start_terrain = get_terrain_at(start);
    if (start_terrain != nullptr) effective_speed += start_terrain->get_speed_modifier(unit);
    effective_speed = (std::max)(0, effective_speed);
    int max_steps = (std::min)(requested, effective_speed);
    if (max_steps == 0)
    {
        error_msg = "Юнит не может покинуть текущую клетку!";
        return false;
    }

    COORD destination = start;
    NeutralObject* encountered = nullptr;
    bool obstacle = false;
    for (int step = 1; step <= max_steps; ++step)
    {
        COORD next{ static_cast<SHORT>(start.X + dir_x * step), static_cast<SHORT>(start.Y + dir_y * step) };
        if (!is_inside_field(next) || is_unit_at(next, index) || is_base_at(next))
        {
            obstacle = true;
            break;
        }
        const Terrain* terrain = get_terrain_at(next);
        if (terrain != nullptr && !terrain->is_passable(unit))
        {
            obstacle = true;
            break;
        }
        destination = next;
        for (const auto& object : neutral_objects)
        {
            if (object != nullptr && object->is_active() && object->get_position().X == next.X && object->get_position().Y == next.Y)
            {
                encountered = object.get();
                break;
            }
        }
        if (encountered != nullptr) break;
    }

    if (destination.X == start.X && destination.Y == start.Y)
    {
        error_msg = "Перед юнитом находится препятствие!";
        return false;
    }

    unit.move_to(destination);
    last_move_distance = std::abs(destination.X - start.X) + std::abs(destination.Y - start.Y);
    update_terrain_bonus(unit);

    if (encountered != nullptr)
    {
        std::shared_ptr<Base> owner = find_owning_base(unit);
        if (owner != nullptr && activate_neutral_object(*encountered, unit, *owner))
        {
            add_log("Юнит [" + std::to_string(index + 1) + "] взаимодействовал с нейтральным объектом!");
            neutral_objects.erase(std::remove_if(neutral_objects.begin(), neutral_objects.end(),
                [](const std::unique_ptr<NeutralObject>& object) { return object == nullptr || !object->is_active(); }), neutral_objects.end());
        }
        else if (owner == nullptr)
        {
            add_log("У юнита не найдена принадлежащая ему база!");
        }
    }

    error_msg = obstacle ? "Юнит остановился перед препятствием." : std::string();
    return true;
}

bool GameField::find_next_step_towards(const Unit& unit, COORD target, COORD& next_step) const
{
    COORD start = unit.get_position();
    if (!is_inside_field(start) || !is_inside_field(target)) return false;
    const int total = width * height;
    std::vector<int> parent(total, -1);
    std::queue<COORD> pending;
    int start_index = cell_index(start.X, start.Y, width);
    parent[start_index] = start_index;
    pending.push(start);
    const int dir_x[4] = { 1, -1, 0, 0 };
    const int dir_y[4] = { 0, 0, 1, -1 };
    int goal_index = -1;

    while (!pending.empty())
    {
        COORD current = pending.front();
        pending.pop();
        int distance = (std::max)(std::abs(current.X - target.X), std::abs(current.Y - target.Y));
        if (distance <= unit.get_range_value() && !(current.X == start.X && current.Y == start.Y))
        {
            goal_index = cell_index(current.X, current.Y, width);
            break;
        }
        for (int direction = 0; direction < 4; ++direction)
        {
            COORD next{ static_cast<SHORT>(current.X + dir_x[direction]), static_cast<SHORT>(current.Y + dir_y[direction]) };
            if (!is_inside_field(next)) continue;
            int next_index = cell_index(next.X, next.Y, width);
            if (parent[next_index] != -1) continue;
            if (is_unit_at(next, units.size()) || is_base_at(next)) continue;
            const Terrain* terrain = get_terrain_at(next);
            if (terrain != nullptr && !terrain->is_passable(unit)) continue;
            parent[next_index] = cell_index(current.X, current.Y, width);
            pending.push(next);
        }
    }

    if (goal_index == -1) return false;
    int current_index = goal_index;
    while (parent[current_index] != start_index)
    {
        current_index = parent[current_index];
        if (current_index == parent[current_index]) return false;
    }
    next_step.X = static_cast<SHORT>(current_index % width);
    next_step.Y = static_cast<SHORT>(current_index / width);
    return true;
}

void GameField::add_log(const std::string& message)
{
    action_log.push_back(message);
    if (action_log.size() > 15) action_log.erase(action_log.begin());
}

void GameField::draw() const
{
    system("cls");
    std::cout << "Карта поля:\n";
    for (int x = 0; x < width + 2; ++x) std::cout << "X ";
    std::cout << "\n";

    for (int y = 0; y < height; ++y)
    {
        std::cout << "X ";
        for (int x = 0; x < width; ++x)
        {
            bool found = false;
            for (size_t i = 0; i < units.size(); ++i)
            {
                if (units[i] != nullptr && units[i]->is_alive() && units[i]->get_position().X == x && units[i]->get_position().Y == y)
                {
                    std::cout << i + 1 << ' ';
                    found = true;
                    break;
                }
            }
            if (found) continue;
            for (const auto& base : bases)
            {
                if (base != nullptr && base->is_alive() && base->get_position().X == x && base->get_position().Y == y)
                {
                    std::cout << (base->is_player_base() ? "P " : "B ");
                    found = true;
                    break;
                }
            }
            if (found) continue;
            for (const auto& object : neutral_objects)
            {
                if (object != nullptr && object->is_active() && object->get_position().X == x && object->get_position().Y == y)
                {
                    std::cout << object->get_symbol() << ' ';
                    found = true;
                    break;
                }
            }
            if (found) continue;
            const Terrain* terrain = get_terrain_at(COORD{ static_cast<SHORT>(x), static_cast<SHORT>(y) });
            if (terrain != nullptr) std::cout << terrain->get_symbol() << ' ';
            else std::cout << "* ";
        }
        std::cout << "X\n";
    }
    for (int x = 0; x < width + 2; ++x) std::cout << "X ";

    std::cout << "\n\nЖивые юниты:\n";
    for (size_t i = 0; i < units.size(); ++i)
    {
        if (units[i] == nullptr || !units[i]->is_alive()) continue;
        const Unit& unit = *units[i];
        std::cout << '[' << i + 1 << "] " << (unit.is_player_unit() ? "Игрок" : "Бот")
            << " | " << get_type_name(unit.get_type())
            << " | HP: " << unit.get_health_value()
            << " | Armor: " << unit.get_armor_value()
            << " | Attack: " << unit.get_attack_value()
            << " | Range: " << unit.get_range_value()
            << " | Speed: " << unit.get_speed_value()
            << " | Position: (" << unit.get_position().X << ", " << unit.get_position().Y << ")\n";
    }

    std::cout << "\nБазы:\n";
    for (const auto& base : bases)
    {
        if (base == nullptr) continue;
        std::cout << (base->is_player_base() ? "База игрока" : "База бота")
            << " | HP: " << base->get_health_value()
            << (base->is_alive() ? "" : " [УНИЧТОЖЕНА]") << '\n';
    }

    std::cout << "\nЖурнал событий:\n";
    for (const auto& message : action_log) std::cout << message << '\n';
}
