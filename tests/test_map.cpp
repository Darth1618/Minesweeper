#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "map.h"

TEST_CASE("Поле инициализируется пустым") {
    Map map(9, 10);
    map.initEmpty();

    CHECK(map.getSize() == 9);
    CHECK(map.getCell(1, 1) == 0);   // ячейка пустая
    CHECK(map.getMask(1, 1) == 0);   // ячейка закрыта
}

TEST_CASE("После расстановки мин — мины есть") {
    Map map(9, 10);
    map.initEmpty();
    map.initMines(5, 5);

    int mineCount = 0;
    for (int y = 1; y < map.getSize()-1; y++)
        for (int x = 1; x < map.getSize()-1; x++)
            if (map.getCell(x, y) == 10)
                mineCount++;

    CHECK(mineCount == 10);
}

TEST_CASE("Первый клик не попадает на мину") {
    Map map(9, 10);
    map.initEmpty();
    map.initMines(1, 1);  // первый клик в (1,1)

    // вся зона 3x3 вокруг (1,1) должна быть без мин
    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++) {
            int nx = 1 + dx, ny = 1 + dy;
            if (nx >= 1 && ny >= 1 && nx < map.getSize()-1 && ny < map.getSize()-1)
                CHECK(map.getCell(nx, ny) != 10);
        }
}

TEST_CASE("Флаг ставится и снимается") {
    Map map(9, 10);
    map.initEmpty();

    map.toggleFlag(3, 3);
    CHECK(map.getMask(3, 3) == 2);   // флаг стоит

    map.toggleFlag(3, 3);
    CHECK(map.getMask(3, 3) == 0);   // флаг снят
}

TEST_CASE("Открытая ячейка не помечается флагом") {
    Map map(9, 10);
    map.initEmpty();
    map.initMines(5, 5);
    map.openCell(3, 3);              // открываем ячейку

    map.toggleFlag(3, 3);            // пытаемся поставить флаг
    CHECK(map.getMask(3, 3) != 2);   // флаг не должен появиться
}