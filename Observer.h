#pragma once
#include "EventType.h"

class Unit;

class Observer
{
public:
    virtual ~Observer() = default;

    virtual void on_unit_event(Unit& unit, EventType event) = 0;
};
