#include <SFML/Graphics.hpp>
#include <ctime>
#include <memory>
#include "Map.h"
using namespace std;

struct Difficulty {
    string name;
    int size;
    int mines;
    int winW;
    int winH;
    int cellSize;
};

const Difficulty LEVELS[3] = {
    {"Junior League", 9, 10, 750, 750, 62},
    {"Beer League", 16, 40, 1200, 1200, 62},
    {"Professional League", 22, 99, 1600, 1600, 62}
};

enum class State { MENU, GAME, GAMEOVER, WIN }; // возможные состояния игры

sf::Color digitColor(int val) {
    switch (val) {
        case 1: return sf::Color(0, 0, 255);
        case 2: return sf::Color(0, 128, 0);
        case 3: return sf::Color(255, 0, 0);
        case 4: return sf::Color(0, 0, 128);
        case 5: return sf::Color(128, 0, 0);
        case 6: return sf::Color(0, 128, 128);
        case 7: return sf::Color(0, 0, 0);
        case 8: return sf::Color(128, 128, 128);
    }
} // цвета ячеек, рядом с которыми бомба

void drawCell(sf::RenderWindow &window, sf::Font &font,
              Map *map, int x, int y,
              float px, float py, int cellSize,
              bool reveal = false)
// xy - игровые координаты ячейки, pxpy - пиксельные координаты на экране, cellSize - размер Ячейки в пикселях, reveal - показ всех мин
{
    sf::RectangleShape cell(sf::Vector2f(cellSize - 2, cellSize - 2));
    cell.setPosition({px, py}); // создание квадрата, где будет ячейка на 2 пикселя меньше, чтобы был зазор

    int maskVal = map->getMask(x, y); // видит игрок: 0 - закрыто, 1 - открыто, 2 - флаг
    int mapVal = map->getCell(x, y); // реальность: 0 - пусто, 1-8 цифра, 10 - мина

    if (reveal && mapVal == 10) {
        cell.setFillColor(sf::Color(220, 50, 50));
        window.draw(cell);
        sf::Text mine(font, "*", cellSize - 4);
        mine.setFillColor(sf::Color::Black);
        mine.setStyle(sf::Text::Bold);
        sf::FloatRect mb = mine.getLocalBounds(); // узнаём размеры мины
        mine.setOrigin({
            mb.position.x + mb.size.x / 2.f,
            mb.position.y + mb.size.y / 2.f
        }); // точку привязки в центр мины
        mine.setPosition({px + cellSize / 2.f, py + cellSize / 2.f}); // ставим мину в центр ячейки
        window.draw(mine);
        return;
    } // показ мины при проигрыше

    if (maskVal == 0) {
        cell.setFillColor(sf::Color(100, 120, 200));
        window.draw(cell);
    } // закрытая ячейка
    else if (maskVal == 2) {
        cell.setFillColor(sf::Color(80, 100, 180));
        cell.setOutlineColor(sf::Color(60, 80, 160));
        cell.setOutlineThickness(1);
        window.draw(cell);
        sf::Text flag(font, "!", cellSize - 4);
        flag.setFillColor(sf::Color::Red);
        flag.setStyle(sf::Text::Bold);
        sf::FloatRect fb = flag.getLocalBounds();
        flag.setOrigin({
            fb.position.x + fb.size.x / 2.f,
            fb.position.y + fb.size.y / 2.f
        });
        flag.setPosition({px + cellSize / 2.f, py + cellSize / 2.f});
        window.draw(flag); // тоже самое центрируем как и с миной
    } // состояние когда флаг
    else {
        if (mapVal == 10) {
            cell.setFillColor(sf::Color(220, 50, 50));
            window.draw(cell);
            sf::Text mine(font, "*", cellSize - 4);
            mine.setFillColor(sf::Color::Black);
            mine.setStyle(sf::Text::Bold);
            sf::FloatRect mb = mine.getLocalBounds();
            mine.setOrigin({
                mb.position.x + mb.size.x / 2.f,
                mb.position.y + mb.size.y / 2.f
            });
            mine.setPosition({px + cellSize / 2.f, py + cellSize / 2.f});
            window.draw(mine); // когда открываем мину рисуем её
        } else {
            cell.setFillColor(sf::Color(210, 210, 210));
            cell.setOutlineColor(sf::Color(180, 180, 180));
            cell.setOutlineThickness(1);
            window.draw(cell); // рисуем пустую клетку
            if (mapVal >= 1 && mapVal <= 8) {
                sf::Text num(font, to_string(mapVal), cellSize - 6);
                num.setFillColor(digitColor(mapVal));
                num.setStyle(sf::Text::Bold);
                sf::FloatRect nb = num.getLocalBounds();
                num.setOrigin({
                    nb.position.x + nb.size.x / 2.f,
                    nb.position.y + nb.size.y / 2.f
                });
                num.setPosition({px + cellSize / 2.f, py + cellSize / 2.f});
                window.draw(num); // рисуем цифру, если нужна
            }
        }
    } // открытая ячейка
}

bool checkWin(Map *map) {
    int sz = map->getSize(); // получаем размеры поля
    for (int y = 1; y < sz - 1; y++)
        for (int x = 1; x < sz - 1; x++)
            if (map->getCell(x, y) != 10 && map->getMask(x, y) == 0)
                return false;
    return true;
} // происходит проверка на все открытые клетки если ещё не открыты, победы нет. Вызывается каждым кликом

void drawField(sf::RenderWindow &window, sf::Font &font,
               Map *map, int offsetX, int offsetY,
               int cellSize, bool reveal = false) {
    int sz = map->getSize();
    for (int y = 1; y < sz - 1; y++)
        for (int x = 1; x < sz - 1; x++)
            drawCell(window, font, map, x, y,
                     offsetX + x * cellSize,
                     offsetY + y * cellSize,
                     cellSize, reveal);
} // рисуем поле с помощью ячеек, offset - отступ сверху и слева от края окна, чтобы не рисовать прямо у края

void resizeWindow(sf::RenderWindow &window, int w, int h) {
    window.close();
    window.create(sf::VideoMode({(unsigned) w, (unsigned) h}),
                  "Minesweeper",
                  sf::Style::Titlebar | sf::Style::Close); // без кнопки развёртки, чтобы игрок не смог сам развернуть
    window.setFramerateLimit(60);
} // в зависимости от уровня, создаётся окно нужного размера

int main() {
    srand(time(NULL));

    int curW = 700, curH = 700;

    sf::RenderWindow window(
        sf::VideoMode({(unsigned) curW, (unsigned) curH}),
        "Minesweeper",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) return -1;

    State state = State::MENU; // начинаем с меню
    unique_ptr<Map> map = nullptr; // поля ещё нет
    int cellSize = 50;
    int offsetX = 30; // отступы от края
    int offsetY = 110; // отстуры от края
    bool firstClick = true; // чтобы не попадать случайно с первого же клика на мину
    int levelIdx = 0; // LEVELS[levelIdx]

    sf::Clock gameClock; // таймер игры
    float elapsedSeconds = 0.f;
    bool timerRunning = false;
    float finalSeconds = 0.f;

    sf::Clock returnClock; // таймер автовозврата в меню
    bool returnTimerActive = false;

    auto goToMenu = [&]() {
        resizeWindow(window, 700, 700);
        curW = 700;
        curH = 700;
    }; // возвращаемся в меню

    while (window.isOpen()) {
        if (timerRunning)
            elapsedSeconds = gameClock.getElapsedTime().asSeconds();

        if (returnTimerActive) {
            float left = 5.f - returnClock.getElapsedTime().asSeconds();
            if (left <= 0.f) {
                returnTimerActive = false;
                map = nullptr;
                state = State::MENU;
                goToMenu();
            }
        } // автовозврат в меню

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close(); // закрытие окна при нажатии на крестик

            if (const auto *e = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2i mouse = e->position; // координаты мыши

                if (state == State::MENU) {
                    for (int i = 0; i < 3; i++) {
                        sf::FloatRect btnRect(
                            {curW / 2.f - 160.f, 220.f + i * 120.f},
                            {320.f, 80.f}
                        ); // воссаздаются кнопки во время проверки клика
                        if (btnRect.contains(sf::Vector2f(mouse))) {
                            levelIdx = i;
                            map = make_unique<Map>(LEVELS[i].size, LEVELS[i].mines);
                            map->initEmpty(); // создаёт поле без мин
                            firstClick = true;
                            timerRunning = false;
                            elapsedSeconds = 0.f;
                            returnTimerActive = false;
                            cellSize = LEVELS[i].cellSize;
                            int fieldWidth = (LEVELS[i].size - 2) * cellSize;
                            offsetX = (LEVELS[i].winH - fieldWidth) / 2 - cellSize;
                            offsetY = 160;
                            curW = LEVELS[i].winW;
                            curH = LEVELS[i].winH;
                            resizeWindow(window, curW, curH); // пересоздаётся окно под уровень
                            state = State::GAME; // переходим к игре
                        } // проверяет, попал ли клик в прямоугольник кнопки
                    }
                } else if (state == State::GAME && map) {
                    int cx = (mouse.x - offsetX) / cellSize;
                    int cy = (mouse.y - offsetY) / cellSize; // рассчёт координат ячейки
                    if (cx >= 1 && cy >= 1 &&
                        cx < map->getSize() - 1 &&
                        cy < map->getSize() - 1) {
                        // проверка клик попал в пределы видимого поля
                        if (e->button == sf::Mouse::Button::Right) {
                            map->toggleFlag(cx, cy);
                        } else if (e->button == sf::Mouse::Button::Left) {
                            if (firstClick) {
                                map->initMines(cx, cy); // мины ставялся избегая cx cy
                                firstClick = false;
                                timerRunning = true;
                                gameClock.restart();
                            }
                            if (!map->openCell(cx, cy)) {
                                timerRunning = false;
                                finalSeconds = elapsedSeconds;
                                returnTimerActive = true;
                                returnClock.restart();
                                state = State::GAMEOVER; // проигрыш, так как мина
                            } else if (checkWin(map.get())) {
                                timerRunning = false;
                                finalSeconds = elapsedSeconds;
                                returnTimerActive = true;
                                returnClock.restart();
                                state = State::WIN;
                            } // проверка на все открытые безопасные ячейки
                        }
                    }
                } else if (state == State::GAMEOVER || state == State::WIN) {
                    returnTimerActive = false;
                    map = nullptr;
                    state = State::MENU;
                    goToMenu();
                } // состояние проигрыш или выигрыш
            } // то есть проверяет является ли событие нужного типа то есть нажатие мышкой
        } // опрос событий

        window.clear(sf::Color(30, 30, 40)); // чтобы рисовать новые кадры и они не заливась друг на друга

        // МЕНЮ
        if (state == State::MENU) {
            sf::Text title(font, "Minesweeper", 80);
            title.setFillColor(sf::Color::White);
            title.setStyle(sf::Text::Bold);
            {
                sf::FloatRect tb = title.getLocalBounds();
                title.setOrigin({tb.position.x + tb.size.x / 2.f, 0.f});
                title.setPosition({curW / 2.f, 50.f});
            } // центрование горизонтальное
            window.draw(title); // отрисовка заголовка названия игры

            sf::Text subtitle(font, "Choose difficulty", 32);
            subtitle.setFillColor(sf::Color(180, 180, 180));
            {
                sf::FloatRect sb = subtitle.getLocalBounds();
                subtitle.setOrigin({sb.position.x + sb.size.x / 2.f, 0.f});
                subtitle.setPosition({curW / 2.f, 160.f});
            } // центрование горизонтальное
            window.draw(subtitle); // отрисовка Выбора Уровня

            string descs[3] = {
                "9x9  |  10 mines",
                "16x16  |  40 mines",
                "22x22  |  99 mines"
            }; // описание кнопок

            for (int i = 0; i < 3; i++) {
                float bx = curW / 2.f - 160.f;
                float by = 220.f + i * 120.f;

                sf::RectangleShape btn({320.f, 100.f});
                btn.setPosition({bx, by});
                btn.setFillColor(sf::Color(70, 130, 180));
                btn.setOutlineColor(sf::Color::White);
                btn.setOutlineThickness(2);
                window.draw(btn);

                sf::Text lbl(font, LEVELS[i].name, 36);
                lbl.setFillColor(sf::Color::White);
                lbl.setStyle(sf::Text::Bold);
                {
                    sf::FloatRect lb = lbl.getLocalBounds();
                    lbl.setOrigin({
                        lb.position.x + lb.size.x / 2.f,
                        lb.position.y + lb.size.y / 2.f
                    });
                    lbl.setPosition({bx + 160.f, by + 28.f});
                }
                window.draw(lbl);

                sf::Text desc(font, descs[i], 26);
                desc.setFillColor(sf::Color(0, 0, 0));
                {
                    sf::FloatRect db = desc.getLocalBounds();
                    desc.setOrigin({db.position.x + db.size.x / 2.f, 0.f});
                    desc.setPosition({bx + 160.f, by + 55.f});
                }
                window.draw(desc);
            } // рисование трёх кнопок
        }

        // ИГРА
        else if (state == State::GAME && map) {
            sf::Text timerText(font,
                               "Time: " + to_string((int) elapsedSeconds) + "s", 46);
            timerText.setFillColor(sf::Color::White);
            timerText.setPosition({(float) offsetX, 60.f});
            window.draw(timerText);

            drawField(window, font, map.get(), offsetX, offsetY, cellSize, false); // вызов рисовки поля
        }

        // ПОРАЖЕНИЕ
        else if (state == State::GAMEOVER) {
            drawField(window, font, map.get(), offsetX, offsetY, cellSize, true);
            // поле с открытыми ячейками и показ мин

            float left = 5.f - returnClock.getElapsedTime().asSeconds();
            if (left < 0.f) left = 0.f; // обратный отсчёт

            sf::Text res(font, "GAME OVER", 64);
            res.setFillColor(sf::Color::Red);
            res.setStyle(sf::Text::Bold);
            {
                sf::FloatRect rb = res.getLocalBounds();
                res.setOrigin({
                    rb.position.x + rb.size.x / 2.f,
                    rb.position.y + rb.size.y / 2.f
                });
                res.setPosition({curW / 2.f, 68.f});
            }
            window.draw(res);

            sf::Text timeT(font,
                           "Your time: " + to_string((int) finalSeconds) + "s", 42);
            timeT.setFillColor(sf::Color::Yellow);
            {
                sf::FloatRect tb = timeT.getLocalBounds();
                timeT.setOrigin({tb.position.x + tb.size.x / 2.f, 0.f});
                timeT.setPosition({curW / 2.f, 98.f});
            }
            window.draw(timeT);

            sf::Text cd(font,
                        "Return to menu in " + to_string((int) left + 1) + "s...", 32);
            cd.setFillColor(sf::Color(180, 180, 180));
            {
                sf::FloatRect cb = cd.getLocalBounds();
                cd.setOrigin({cb.position.x + cb.size.x / 2.f, 0.f});
                cd.setPosition({curW / 2.f, 148.f});
            }
            window.draw(cd);
        }

        // ПОБЕДА
        else if (state == State::WIN) {
            drawField(window, font, map.get(), offsetX, offsetY, cellSize, false);

            float left = 5.f - returnClock.getElapsedTime().asSeconds();
            if (left < 0.f) left = 0.f;

            sf::Text res(font, "YOU WIN!", 64);
            res.setFillColor(sf::Color(0, 220, 0));
            res.setStyle(sf::Text::Bold);
            {
                sf::FloatRect rb = res.getLocalBounds();
                res.setOrigin({
                    rb.position.x + rb.size.x / 2.f,
                    rb.position.y + rb.size.y / 2.f
                });
                res.setPosition({curW / 2.f, 68.f});
            }
            window.draw(res);

            sf::Text timeT(font,
                           "Your time: " + to_string((int) finalSeconds) + "s", 42);
            timeT.setFillColor(sf::Color::Yellow);
            {
                sf::FloatRect tb = timeT.getLocalBounds();
                timeT.setOrigin({tb.position.x + tb.size.x / 2.f, 0.f});
                timeT.setPosition({curW / 2.f, 98.f});
            }
            window.draw(timeT);

            sf::Text cd(font,
                        "Return to menu in " + to_string((int) left + 1) + "s...", 32);
            cd.setFillColor(sf::Color(180, 180, 180));
            {
                sf::FloatRect cb = cd.getLocalBounds();
                cd.setOrigin({cb.position.x + cb.size.x / 2.f, 0.f});
                cd.setPosition({curW / 2.f, 148.f});
            }
            window.draw(cd);
        }
        window.display(); // показ каждого кадра после всех операций
    }
    return 0;
}
