#include "Unit.h"

#include <algorithm>
#include <cmath>

Unit::Unit(COORD pos, int hp, int arm, int atk, int rng, int s, UnitType t, bool player)
    : position(pos), health(hp), armor(arm), attack(atk), range(rng), speed(s), type(t),
      is_player(player), death_logged(false), death_notified(false), terrain_armor_bonus(0)
{
}

Unit::Unit(const Unit& other)
    : position(other.position), health(other.health), armor(other.armor), attack(other.attack),
      range(other.range), speed(other.speed), type(other.type), is_player(other.is_player),
      death_logged(other.death_logged), death_notified(other.death_notified),
      terrain_armor_bonus(other.terrain_armor_bonus), observers()
{
}

bool Unit::is_alive() const { return !health.is_zero(); }
UnitType Unit::get_type() const { return type; }

void Unit::add_observer(const std::shared_ptr<Observer>& observer)
{
    if (observer == nullptr) return;
    remove_expired_observers();
    for (const auto& weak_observer : observers)
        if (weak_observer.lock() == observer) return;
    observers.push_back(observer);
    observer->on_unit_event(*this, EventType::Created);
}

void Unit::remove_expired_observers()
{
    observers.erase(std::remove_if(observers.begin(), observers.end(),
        [](const std::weak_ptr<Observer>& observer) { return observer.expired(); }), observers.end());
}

void Unit::notify_observers(EventType event)
{
    for (auto it = observers.begin(); it != observers.end();)
    {
        std::shared_ptr<Observer> observer = it->lock();
        if (observer != nullptr) { observer->on_unit_event(*this, event); ++it; }
        else it = observers.erase(it);
    }
}

bool Unit::can_attack(COORD target_pos) const
{
    if (!is_alive()) return false;
    int distance = (std::max)(std::abs(position.X - target_pos.X), std::abs(position.Y - target_pos.Y));
    return distance <= range.get_range();
}

void Unit::take_damage(int damage)
{
    if (!is_alive() || damage <= 0) return;
    health.decrease(damage);
    notify_observers(EventType::Damaged);
    if (!is_alive() && !death_notified)
    {
        death_notified = true;
        notify_observers(EventType::Destroyed);
    }
}

void Unit::take_healing(int amount)
{
    if (is_alive() && amount > 0) health.increase(amount);
}

bool Unit::can_move_to(COORD target_pos) const
{
    if (!is_alive()) return false;
    int dx = std::abs(position.X - target_pos.X);
    int dy = std::abs(position.Y - target_pos.Y);
    if (dx > 0 && dy > 0) return false;
    int distance = dx + dy;
    return distance > 0 && distance <= speed.get_speed();
}

void Unit::move_to(COORD new_pos)
{
    if (can_move_to(new_pos)) position = new_pos;
}
