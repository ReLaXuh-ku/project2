#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <string>
#include <limits>
#include <algorithm>
#include <clocale>
#include <Windows.h>

#include "GameField.h"
#include "UnitFactory.h"
#include "NeutralObjectFactory.h"

namespace
{
    void clear_input()
    {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    }

    int read_int(const std::string& prompt, int min_value, int max_value)
    {
        int value = 0;

        while (true)
        {
            std::cout << prompt;

            if (std::cin >> value &&
                value >= min_value &&
                value <= max_value)
            {
                return value;
            }

            std::cout << "Некорректный ввод. Повторите попытку.\n";
            clear_input();
        }
    }

    UnitType unit_type_from_choice(int choice)
    {
        switch (choice)
        {
        case 1: return UnitType::bruiser;
        case 2: return UnitType::samurai;
        case 3: return UnitType::soldier;
        case 4: return UnitType::mage;
        case 5: return UnitType::healer;
        case 6: return UnitType::cannoneer;
        default: return UnitType::bruiser;
        }
    }

    const char* unit_name(UnitType type)
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

    bool create_unit_near_base(
        GameField& field,
        const std::shared_ptr<Base>& base,
        UnitType type)
    {
        if (base == nullptr || !base->is_alive())
        {
            return false;
        }

        const COORD base_pos = base->get_position();

        const int dx[8] = { 1, 1, 1, 0, 0, -1, -1, -1 };
        const int dy[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };

        for (int i = 0; i < 8; ++i)
        {
            COORD candidate
            {
                static_cast<SHORT>(base_pos.X + dx[i]),
                static_cast<SHORT>(base_pos.Y + dy[i])
            };

            if (field.create_unit_for_base(base, type, candidate))
            {
                return true;
            }
        }

        return false;
    }

    bool player_action(GameField& field)
    {
        int unit_number = read_int(
            "Выберите своего юнита (номер на карте): ",
            1,
            static_cast<int>(field.get_units_count()));

        size_t index = static_cast<size_t>(unit_number - 1);

        Unit& actor = field.get_unit_by_index(index);

        if (!actor.is_alive() || !actor.is_player_unit())
        {
            std::cout << "Выбранный юнит погиб или принадлежит противнику. "
                "Очко действия не потрачено.\n";

            return false;
        }

        int target_number = read_int(
            "Цель: 0 — база бота, 1..N — номер юнита: ",
            0,
            static_cast<int>(field.get_units_count()));

        ICombatTarget* target = nullptr;

        if (target_number == 0)
        {
            if (actor.get_type() == UnitType::healer)
            {
                std::cout << "Хилер не может лечить вражескую базу.\n";
                return false;
            }

            std::shared_ptr<Base> enemy_base = field.get_base(false);

            if (enemy_base == nullptr || !enemy_base->is_alive())
            {
                std::cout << "База противника уже уничтожена.\n";
                return false;
            }

            target = enemy_base.get();
        }
        else
        {
            Unit& target_unit = field.get_unit_by_index(
                static_cast<size_t>(target_number - 1));

            if (!target_unit.is_alive())
            {
                std::cout << "Цель уже погибла.\n";
                return false;
            }

            if (actor.get_type() == UnitType::healer)
            {
                if (!target_unit.is_player_unit())
                {
                    std::cout << "Хилер может лечить только союзников.\n";
                    return false;
                }
            }
            else if (target_unit.is_player_unit())
            {
                std::cout << "Атаковать союзников нельзя.\n";
                return false;
            }

            target = &target_unit;
        }

        if (!actor.can_attack(target->get_position()))
        {
            std::cout << "Цель вне радиуса действия. "
                "Очко действия не потрачено.\n";

            return false;
        }

        int old_health = target->get_health_value();

        actor.execute_action(*target);

        field.add_log(
            "Игрок: " +
            std::string(unit_name(actor.get_type())) +
            " применил действие.");

        field.process_deaths();

        if (actor.get_type() == UnitType::healer &&
            target->get_health_value() == old_health)
        {
            std::cout << "Цель не получила лечения "
                "(возможно, здоровье уже максимальное).\n";
        }

        return true;
    }

    bool player_move(GameField& field)
    {
        int number = read_int(
            "Выберите своего юнита (номер на карте): ",
            1,
            static_cast<int>(field.get_units_count()));

        size_t index = static_cast<size_t>(number - 1);

        Unit& unit = field.get_unit_by_index(index);

        if (!unit.is_alive() || !unit.is_player_unit())
        {
            std::cout << "Выбранный юнит погиб или принадлежит противнику.\n";
            return false;
        }

        char direction = 0;

        std::cout << "Направление (W/A/S/D): ";
        std::cin >> direction;

        if (direction >= 'a' && direction <= 'z')
        {
            direction = static_cast<char>(direction - 'a' + 'A');
        }

        if (direction != 'W' &&
            direction != 'A' &&
            direction != 'S' &&
            direction != 'D')
        {
            std::cout << "Неверное направление.\n";
            return false;
        }

        int steps = read_int(
            "Количество клеток (1-" +
            std::to_string(unit.get_speed_value()) + "): ",
            1,
            unit.get_speed_value());

        COORD destination = unit.get_position();

        if (direction == 'W')
            destination.Y = static_cast<SHORT>(destination.Y - steps);

        if (direction == 'S')
            destination.Y = static_cast<SHORT>(destination.Y + steps);

        if (direction == 'A')
            destination.X = static_cast<SHORT>(destination.X - steps);

        if (direction == 'D')
            destination.X = static_cast<SHORT>(destination.X + steps);

        std::string error;

        if (!field.move_unit_by_index(index, destination, error))
        {
            std::cout << error << " Очко действия не потрачено.\n";
            return false;
        }

        field.add_log(
            "Игрок: юнит [" +
            std::to_string(number) +
            "] переместился на " +
            std::to_string(field.get_last_move_distance()) +
            " клеток.");

        if (!error.empty())
        {
            std::cout << error << '\n';
        }

        field.process_deaths();

        return true;
    }

    bool player_create(GameField& field)
    {
        std::shared_ptr<Base> base = field.get_base(true);

        if (base == nullptr || !base->is_alive())
        {
            std::cout << "Ваша база уничтожена — создавать юнитов нельзя.\n";
            return false;
        }

        if (field.get_alive_units_count(true) >= 3)
        {
            std::cout << "Лимит в три живых юнита уже достигнут.\n";
            return false;
        }

        int type_choice = read_int(
            "Класс: 1-Брузер 2-Самурай 3-Солдат "
            "4-Маг 5-Хилер 6-Канонир: ",
            1,
            6);

        UnitType type = unit_type_from_choice(type_choice);

        if (!create_unit_near_base(field, base, type))
        {
            std::cout << "Нет подходящей свободной клетки рядом с базой.\n";
            return false;
        }

        field.add_log(
            "Игрок создал юнита: " +
            std::string(unit_name(type)) +
            ".");

        return true;
    }

    bool bot_can_act(GameField& field, size_t bot_index)
    {
        Unit& bot = field.get_unit_by_index(bot_index);

        if (!bot.is_alive() || bot.is_player_unit())
        {
            return false;
        }

        if (bot.get_type() == UnitType::healer)
        {
            Unit* injured = nullptr;
            int lowest_health = (std::numeric_limits<int>::max)();

            for (const auto& ally_ptr : field.get_all_units())
            {
                if (ally_ptr == nullptr ||
                    !ally_ptr->is_alive() ||
                    ally_ptr->is_player_unit())
                {
                    continue;
                }

                if (!bot.can_attack(ally_ptr->get_position()))
                {
                    continue;
                }

                int current_health = ally_ptr->get_health_value();

                if (current_health < lowest_health)
                {
                    injured = ally_ptr.get();
                    lowest_health = current_health;
                }
            }

            if (injured != nullptr)
            {
                bot.execute_action(*injured);

                field.add_log(
                    "Бот: целитель [" +
                    std::to_string(bot_index + 1) +
                    "] восстановил здоровье союзнику.");

                return true;
            }

            return false;
        }

        for (size_t i = 0; i < field.get_units_count(); ++i)
        {
            Unit& target = field.get_unit_by_index(i);

            if (target.is_alive() &&
                target.is_player_unit() &&
                bot.can_attack(target.get_position()))
            {
                bot.execute_action(target);

                field.add_log(
                    "Бот: юнит [" +
                    std::to_string(bot_index + 1) +
                    "] атаковал юнита [" +
                    std::to_string(i + 1) +
                    "].");

                field.process_deaths();

                return true;
            }
        }

        std::shared_ptr<Base> player_base = field.get_base(true);

        if (player_base != nullptr &&
            player_base->is_alive() &&
            bot.can_attack(player_base->get_position()))
        {
            bot.execute_action(*player_base);
            field.add_log("Бот атаковал базу игрока.");

            return true;
        }

        return false;
    }

    bool bot_move(GameField& field, size_t bot_index)
    {
        Unit& bot = field.get_unit_by_index(bot_index);

        if (!bot.is_alive() || bot.is_player_unit())
        {
            return false;
        }

        COORD target{};
        bool has_target = false;

        int best_distance = (std::numeric_limits<int>::max)();

        for (const auto& player_unit : field.get_all_units())
        {
            if (player_unit == nullptr ||
                !player_unit->is_alive() ||
                !player_unit->is_player_unit())
            {
                continue;
            }

            COORD pos = player_unit->get_position();

            int distance =
                abs(pos.X - bot.get_position().X) +
                abs(pos.Y - bot.get_position().Y);

            if (distance < best_distance)
            {
                best_distance = distance;
                target = pos;
                has_target = true;
            }
        }

        if (!has_target)
        {
            std::shared_ptr<Base> player_base = field.get_base(true);

            if (player_base != nullptr && player_base->is_alive())
            {
                target = player_base->get_position();
                has_target = true;
            }
        }

        if (!has_target)
        {
            return false;
        }

        COORD next{};

        if (!field.find_next_step_towards(bot, target, next))
        {
            return false;
        }

        std::string error;

        if (!field.move_unit_by_index(bot_index, next, error))
        {
            return false;
        }

        field.add_log(
            "Бот переместил юнита [" +
            std::to_string(bot_index + 1) +
            "] по найденному маршруту.");

        field.process_deaths();

        return true;
    }

    void bot_turn(GameField& field)
    {
        std::cout << "\nХодит бот...\n";
        Sleep(500);

        int action_points = 3;

        while (action_points > 0)
        {
            bool action_done = false;

            // Сначала бот пытается создать юнита,
            // если лимит ещё не достигнут.
            if (field.get_alive_units_count(false) < 3)
            {
                std::shared_ptr<Base> base = field.get_base(false);

                if (base != nullptr && base->is_alive())
                {
                    UnitType random_type =
                        static_cast<UnitType>(std::rand() % 6);

                    if (create_unit_near_base(field, base, random_type))
                    {
                        field.add_log(
                            "Бот создал юнита: " +
                            std::string(unit_name(random_type)) +
                            ".");

                        action_done = true;
                    }
                }
            }

            // Если создание не удалось, бот пробует атаковать
            // или лечить союзников.
            if (!action_done)
            {
                for (size_t i = 0; i < field.get_units_count(); ++i)
                {
                    if (bot_can_act(field, i))
                    {
                        action_done = true;
                        break;
                    }
                }
            }

            // Если атаковать некого, бот ищет путь к противнику.
            if (!action_done)
            {
                for (size_t i = 0; i < field.get_units_count(); ++i)
                {
                    const Unit& candidate =
                        field.get_unit_by_index(i);

                    if (!candidate.is_alive() ||
                        candidate.is_player_unit())
                    {
                        continue;
                    }

                    if (bot_move(field, i))
                    {
                        action_done = true;
                        break;
                    }
                }
            }

            if (!action_done)
            {
                break;
            }

            --action_points;
        }

        if (action_points > 0)
        {
            field.add_log(
                "Бот завершил ход: доступных действий больше нет.");
        }

        field.process_deaths();
    }
}

int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    const int field_width = 10;
    const int field_height = 10;

    GameField field(field_width, field_height, 6);

    // Создаём базы.
    auto player_base = std::make_shared<Base>(
        COORD{ 0, 5 }, 2000, 20, 3, true);

    auto bot_base = std::make_shared<Base>(
        COORD{ 9, 5 }, 2000, 20, 3, false);

    field.add_base(player_base);
    field.add_base(bot_base);

    // Размещаем ландшафт.
    // Скала доступна только самураю.
    // Болото можно покинуть только юнитом со скоростью больше 1.
    // Лес даёт +10 брони.

    bool world_setup_ok = true;

    world_setup_ok =
        field.add_terrain(
            std::make_unique<Mountain>(COORD{ 4, 2 }))
        && world_setup_ok;

    world_setup_ok =
        field.add_terrain(
            std::make_unique<Swamp>(COORD{ 5, 4 }))
        && world_setup_ok;

    world_setup_ok =
        field.add_terrain(
            std::make_unique<Forest>(COORD{ 4, 7 }))
        && world_setup_ok;

    // Размещаем нейтральные объекты.
    // Они срабатывают при входе юнита на соответствующую клетку.

    world_setup_ok =
        field.add_neutral_object(
            NeutralObjectFactory::create_object(
                NeutralObjectType::TreasureChest,
                COORD{ 3, 3 }))
        && world_setup_ok;

    world_setup_ok =
        field.add_neutral_object(
            NeutralObjectFactory::create_object(
                NeutralObjectType::Trap,
                COORD{ 6, 3 }))
        && world_setup_ok;

    world_setup_ok =
        field.add_neutral_object(
            NeutralObjectFactory::create_object(
                NeutralObjectType::PowerFountain,
                COORD{ 3, 6 }))
        && world_setup_ok;

    world_setup_ok =
        field.add_neutral_object(
            NeutralObjectFactory::create_object(
                NeutralObjectType::TrainingCamp,
                COORD{ 6, 6 }))
        && world_setup_ok;

    if (!world_setup_ok)
    {
        std::cout
            << "Предупреждение: не удалось разместить "
            << "один или несколько объектов мира.\n";
    }

    // Создаём стартовых юнитов игрока.
    std::cout
        << "Классы: 1-Брузер | 2-Самурай | 3-Солдат "
        << "| 4-Маг | 5-Хилер | 6-Канонир\n";

    for (int i = 0; i < 3; ++i)
    {
        int choice = read_int(
            "Выберите класс стартового юнита [" +
            std::to_string(i + 1) + "]: ",
            1,
            6);

        COORD spawn
        {
            1,
            static_cast<SHORT>(2 + i * 2)
        };

        if (!field.create_unit_for_base(
            player_base,
            unit_type_from_choice(choice),
            spawn))
        {
            std::cout << "Не удалось разместить стартового юнита.\n";
        }
    }

    // Создаём стартовых юнитов бота.
    for (int i = 0; i < 3; ++i)
    {
        COORD spawn
        {
            8,
            static_cast<SHORT>(2 + i * 2)
        };

        UnitType type =
            static_cast<UnitType>(std::rand() % 6);

        field.create_unit_for_base(bot_base, type, spawn);
    }

    field.add_log(
        "Игра началась! У каждой команды по три очка действий за ход.");

    bool player_turn = true;

    while (true)
    {
        field.process_deaths();
        field.draw();

        bool player_base_alive = player_base->is_alive();
        bool bot_base_alive = bot_base->is_alive();

        size_t player_units =
            field.get_alive_units_count(true);

        size_t bot_units =
            field.get_alive_units_count(false);

        // Победа или поражение наступает только тогда,
        // когда уничтожены база и все юниты команды.

        if (!player_base_alive && player_units == 0)
        {
            std::cout
                << "\nПОРАЖЕНИЕ: уничтожены ваша база "
                << "и все ваши юниты.\n";

            break;
        }

        if (!bot_base_alive && bot_units == 0)
        {
            std::cout
                << "\nПОБЕДА: уничтожены база "
                << "и все юниты противника!\n";

            break;
        }

        if (player_turn)
        {
            std::cout << "\n>>> ВАШ ХОД (3 очка действий) <<<\n";

            int action_points = 3;

            while (action_points > 0)
            {
                field.draw();

                std::cout
                    << "Осталось очков действий: "
                    << action_points << "\n";

                std::cout
                    << "1 — создать юнита | "
                    << "2 — переместить юнита | "
                    << "3 — действие юнита | "
                    << "0 — закончить ход\n";

                int choice = read_int("Ваш выбор: ", 0, 3);

                if (choice == 0)
                {
                    break;
                }

                bool success = false;

                if (choice == 1)
                {
                    success = player_create(field);
                }
                else if (choice == 2)
                {
                    success = player_move(field);
                }
                else if (choice == 3)
                {
                    success = player_action(field);
                }

                if (success)
                {
                    --action_points;
                }
                else
                {
                    std::cout
                        << "Очко действия сохранено — "
                        << "выберите действие повторно.\n";
                }
            }

            player_turn = false;
        }
        else
        {
            bot_turn(field);
            player_turn = true;
        }
    }

    return 0;
}
