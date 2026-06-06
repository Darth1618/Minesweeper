#pragma once
#include <vector>
#include <cstdlib>
using namespace std;

/**
 * @brief Класс игрового поля сапёра.
 *
 * Хранит два слоя: реальное поле (map) и то, что видит игрок (mask).
 * Обеспечивает логику открытия ячеек, расстановки мин и флагов.
 */
class Map {
private:
    const int BORDER = 100; ///< Значение граничной (невидимой) ячейки-буфера
    const int NOTHING = 0; ///< Значение пустой ячейки (0 соседей-мин)
    const int MINE = 10; ///< Значение ячейки с миной
    const int FLAG = 2; ///< Значение флага в маске

    int size; ///< Размер поля включая граничный буфер со всех сторон
    int numMines; ///< Количество мин на поле

    vector<vector<int> > map; ///< Реальное поле: содержит MINE, цифры 1-8, NOTHING, BORDER
    vector<vector<int> > mask; ///< Маска видимости: 0 — закрыта, 1 — открыта, 2 — флаг, BORDER — граница

public:
    /**
     * @brief Конструктор поля.
     * @param size Размер поля включая граничный буфер (реальное поле = size-2 x size-2).
     * @param numMines Количество мин, которые будут расставлены при первом клике.
     */
    Map(int size, int numMines) : size(size), numMines(numMines) {
    }

    /**
     * @brief Инициализирует пустое поле без мин.
     *
     * Очищает оба массива и заполняет их заново:
     * края — BORDER, внутренние ячейки — NOTHING.
     * Вызывается при старте новой игры до первого клика.
     */
    void initEmpty() {
        map.clear();
        mask.clear();
        initMap();
        initMask();
    }

    /**
     * @brief Расставляет мины и вычисляет цифры для всех ячеек.
     *
     * Вызывается после первого клика, чтобы гарантировать
     * что первая открытая ячейка не окажется миной.
     *
     * @param firstX Координата X первого клика. Зона 3x3 вокруг неё защищена от мин.
     * @param firstY Координата Y первого клика. Зона 3x3 вокруг неё защищена от мин.
     */
    void initMines(int firstX, int firstY) {
        setRandMines(numMines, firstX, firstY);
        SetDigits();
    }

    /**
     * @brief Открывает ячейку по клику игрока.
     *
     * Если на ячейке стоит флаг — игнорирует клик.
     * Если под ячейкой мина — возвращает false (проигрыш).
     * Иначе рекурсивно открывает ячейку и соседей через openEmpty().
     *
     * @param x Координата X ячейки в массиве поля.
     * @param y Координата Y ячейки в массиве поля.
     * @return true — ячейка успешно открыта, игра продолжается.
     * @return false — под ячейкой мина, игра окончена.
     */
    bool openCell(int x, int y) {
        if (mask[y][x] == FLAG) return true;
        if (map[y][x] == MINE) return false;
        openEmpty(x, y);
        return true;
    }

    /**
     * @brief Переключает флаг на закрытой ячейке.
     *
     * Если флага нет — ставит его. Если флаг стоит — убирает.
     * На уже открытую ячейку флаг поставить нельзя.
     *
     * @param x Координата X ячейки в массиве поля.
     * @param y Координата Y ячейки в массиве поля.
     */
    void toggleFlag(int x, int y) {
        if (mask[y][x] == 1) return;
        if (mask[y][x] == FLAG)
            mask[y][x] = NOTHING;
        else
            mask[y][x] = FLAG;
    }

    /**
     * @brief Проверяет, является ли ячейка граничной.
     *
     * @param x Координата X проверяемой ячейки.
     * @param y Координата Y проверяемой ячейки.
     * @return true — ячейка граничная (BORDER).
     * @return false — ячейка внутри поля или координаты за пределами массива.
     */
    bool isBorder(int x, int y) {
        if (x < 0 || x >= size || y < 0 || y >= size) return false;
        return map[y][x] == BORDER;
    }

    /**
     * @brief Возвращает полный размер поля включая граничный буфер.
     * @return Размер поля (реальная игровая область = size-2 x size-2).
     */
    int getSize() { return size; }

    /**
     * @brief Возвращает значение ячейки реального поля.
     *
     * Возможные значения: 0 (пусто), 1-8 (количество соседних мин),
     * 10 (мина), 100 (граница).
     *
     * @param x Координата X ячейки.
     * @param y Координата Y ячейки.
     * @return Значение ячейки из массива map.
     */
    int getCell(int x, int y) { return map[y][x]; }

    /**
     * @brief Возвращает значение маски видимости ячейки.
     *
     * Возможные значения: 0 (закрыта), 1 (открыта), 2 (флаг), 100 (граница).
     *
     * @param x Координата X ячейки.
     * @param y Координата Y ячейки.
     * @return Значение ячейки из массива mask.
     */
    int getMask(int x, int y) { return mask[y][x]; }

private:
    /**
     * @brief Создаёт и заполняет массив реального поля.
     *
     * Крайние строки и столбцы заполняются BORDER,
     * внутренние ячейки — NOTHING.
     */
    void initMap() {
        for (int i = 0; i < size; i++) {
            vector<int> temp;
            for (int j = 0; j < size; j++) {
                if (i == 0 || j == 0 || i == size - 1 || j == size - 1)
                    temp.push_back(BORDER);
                else
                    temp.push_back(NOTHING);
            }
            map.push_back(temp);
        }
    }

    /**
     * @brief Создаёт и заполняет массив маски видимости.
     *
     * Структура идентична initMap(): края — BORDER,
     * внутренние ячейки — NOTHING (все ячейки закрыты).
     */
    void initMask() {
        for (int i = 0; i < size; i++) {
            vector<int> temp;
            for (int j = 0; j < size; j++) {
                if (i == 0 || j == 0 || i == size - 1 || j == size - 1)
                    temp.push_back(BORDER);
                else
                    temp.push_back(NOTHING);
            }
            mask.push_back(temp);
        }
    }

    /**
     * @brief Расставляет мины случайным образом.
     *
     * Гарантирует что мины не попадут в зону 3x3 вокруг первого клика
     * и не окажутся в одной ячейке дважды.
     *
     * @param n Количество мин для расстановки.
     * @param fx Координата X первого клика (центр защищённой зоны).
     * @param fy Координата Y первого клика (центр защищённой зоны).
     */
    void setRandMines(int n, int fx, int fy) {
        for (int i = 0; i < n; i++) {
            int x, y;
            do {
                x = rand() % (size - 2) + 1;
                y = rand() % (size - 2) + 1;
            } while (map[y][x] == MINE ||
                     (abs(x - fx) <= 1 && abs(y - fy) <= 1));
            map[y][x] = MINE;
        }
    }

    /**
     * @brief Вычисляет и записывает цифры для всех не-минных ячеек.
     *
     * Для каждой ячейки подсчитывает количество мин среди 8 соседей
     * и записывает результат (0-8) в массив map.
     * Вызывается один раз после расстановки всех мин.
     */
    void SetDigits() {
        for (int i = 1; i < size - 1; i++) {
            for (int j = 1; j < size - 1; j++) {
                if (map[i][j] == MINE) continue;
                int d = 0;
                if (map[i][j + 1] == MINE) d++;
                if (map[i + 1][j] == MINE) d++;
                if (map[i + 1][j + 1] == MINE) d++;
                if (map[i][j - 1] == MINE) d++;
                if (map[i - 1][j] == MINE) d++;
                if (map[i - 1][j - 1] == MINE) d++;
                if (map[i - 1][j + 1] == MINE) d++;
                if (map[i + 1][j - 1] == MINE) d++;
                map[i][j] = d;
            }
        }
    }

    /**
     * @brief Рекурсивно открывает ячейку и её соседей (flood fill).
     *
     * Открывает текущую ячейку, и если она пустая (значение 0) —
     * рекурсивно вызывает себя для всех 8 соседей.
     * Останавливается на ячейках с цифрами, флагами, минах и границах.
     *
     * @param x Координата X открываемой ячейки.
     * @param y Координата Y открываемой ячейки.
     */
    void openEmpty(int x, int y) {
        if (x < 1 || y < 1 || x >= size - 1 || y >= size - 1) return;
        if (mask[y][x] == 1) return;
        if (mask[y][x] == FLAG) return;
        if (map[y][x] == MINE) return;

        mask[y][x] = 1;

        if (map[y][x] == 0) {
            openEmpty(x + 1, y);
            openEmpty(x - 1, y);
            openEmpty(x, y + 1);
            openEmpty(x, y - 1);
            openEmpty(x + 1, y + 1);
            openEmpty(x - 1, y - 1);
            openEmpty(x + 1, y - 1);
            openEmpty(x - 1, y + 1);
        }
    }
};
