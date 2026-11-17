#include "Base.h"

#include <algorithm>

#include "UnitFactory.h"

Base::Base(COORD location, int health, int armor, int max_units_amount, bool is_player)
    : location(location),
      health(health),
      armor(armor),
      max_units_amount(max_units_amount < 0 ? 0 : max_units_amount),
      is_player(is_player)
{
}

void Base::remove_inactive_units()
{
    base_units.erase(
        std::remove_if(base_units.begin(), base_units.end(),
            [](const std::weak_ptr<Unit>& weak_unit)
            {
                std::shared_ptr<Unit> unit = weak_unit.lock();
                return unit == nullptr || !unit->is_alive();
            }),
        base_units.end());
}

int Base::get_alive_units_count() const
{
    int count = 0;
    for (const auto& weak_unit : base_units)
    {
        std::shared_ptr<Unit> unit = weak_unit.lock();
        if (unit != nullptr && unit->is_alive())
        {
            ++count;
        }
    }
    return count;
}

std::shared_ptr<Unit> Base::create_unit(UnitType type, COORD spawn, bool player_team)
{
    remove_inactive_units();

    if (!is_alive() || player_team != is_player ||
        get_alive_units_count() >= max_units_amount)
    {
        return nullptr;
    }

    std::shared_ptr<Unit> new_unit = UnitFactory::create_unit(type, spawn, player_team);
    if (new_unit == nullptr)
    {
        return nullptr;
    }

    base_units.push_back(new_unit);
    new_unit->add_observer(shared_from_this());
    return new_unit;
}

void Base::take_damage(int damage)
{
    if (damage > 0 && is_alive())
    {
        health.decrease(damage);
    }
}

void Base::take_healing(int amount)
{
    if (amount > 0 && is_alive())
    {
        health.increase(amount);
    }
}

void Base::on_unit_event(Unit& unit, EventType event)
{
    if (event == EventType::Destroyed)
    {
        base_units.erase(
            std::remove_if(base_units.begin(), base_units.end(),
                [&unit](const std::weak_ptr<Unit>& weak_unit)
                {
                    std::shared_ptr<Unit> current = weak_unit.lock();
                    return current == nullptr || current.get() == &unit;
                }),
            base_units.end());
    }
}

bool Base::owns_unit(const Unit& unit) const
{
    for (const auto& weak_unit : base_units)
    {
        std::shared_ptr<Unit> current = weak_unit.lock();
        if (current != nullptr && current.get() == &unit && current->is_alive())
        {
            return true;
        }
    }
    return false;
}
