#ifndef PUBLICWINDOW_H
#define PUBLICWINDOW_H

#include <QWidget>
#include <QLabel>

class PublicWindow : public QWidget
{
    Q_OBJECT
public:
    PublicWindow();
    ~PublicWindow();

    QLabel *timerlabel;
};

#endif