#ifndef FENETRE_H
#define FENETRE_H

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QScreen>
#include "Sender.h"
#include <QTimer>

namespace Ui {
class fenetre;
}

class fenetre : public QWidget
{
    Q_OBJECT

public:
    explicit fenetre(QWidget *parent = nullptr);
    ~fenetre();

    QImage shootScreen();
    void sendNewPicture();
    QString getStringState(int state) const;

private slots:
    void onConnect1();
    void onConnect2();
    void onWS1StateChanged(int state);
    void onWS2StateChanged(int state);
    void onTimerChange();
    void onTransformChanged();

private:
    Ui::fenetre *ui;
    QTimer timerUpdate;
    Sender wsSender1;
    Sender wsSender2;
    int layerRotation = 0;
    bool layerMiroirV = false;
    bool layerMiroirH = false;
};

#endif // FENETRE_H
