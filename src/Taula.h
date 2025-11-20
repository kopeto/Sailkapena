#ifndef TAULA_H
#define TAULA_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QSortFilterProxyModel>

#include "Player.h"

enum class GameState;

class MultiSortProxy : public QSortFilterProxyModel {
public:
    explicit MultiSortProxy(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent) {}

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override {
        // columna primaria
        QVariant a1 = sourceModel()->data(sourceModel()->index(left.row(), 0));
        QVariant b1 = sourceModel()->data(sourceModel()->index(right.row(), 0));
        if (a1 != b1)
            return true;

        // columna secundaria
        QVariant a2 = sourceModel()->data(sourceModel()->index(left.row(), 1));
        QVariant b2 = sourceModel()->data(sourceModel()->index(right.row(), 1));
        return  true;
    }
};

class Taula: public QTableWidget
{
    Q_OBJECT

public:
    static constexpr int PLAYER_NAME_COLUMN   = 1;
    static constexpr int GAME1_TIME_COLUMN    = 2;
    static constexpr int GAME1_ERRORS_COLUMN  = 3;
    static constexpr int GAME2_TIME_COLUMN    = 4;
    static constexpr int GAME2_ERRORS_COLUMN  = 5;
    static constexpr int GAME1_2_TIME_COLUMN  = 6;
    static constexpr int GAME1_2_ERRORS_COLUMN  = 7;
    static constexpr int FINALS_SEPARATOR_COLUMN = 8;
    static constexpr int FINALS_TIME_COLUMN    = 9;
    static constexpr int FINALS_ERRORS_COLUMN  = 10;

    Taula(QWidget *parent = nullptr);
    ~Taula() = default;

    void addPlayer(const Player &player);

    QString getPlayerNameFromRow(int row);
    bool checkPlayerExists(const QString& name);

    void deleteRow(int row);

    int getRowTotalTimeInternal(int row);
    int getRowTotalTimeReal(int row);

    int getRowTotalErrorsInternal(int row);
    int getRowTotalErrorsReal(int row);

    void sort(GameState _state);

    void highlightFirstRows(int numRows);

    protected:
    void keyPressEvent(QKeyEvent *event) override ;

};


#endif