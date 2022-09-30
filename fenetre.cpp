#include "fenetre.h"
#include "ui_fenetre.h"

fenetre::fenetre(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::fenetre),
    wsSender1(),
    wsSender2()
{
    ui->setupUi(this);
    connect(&timerUpdate, &QTimer::timeout, this, &fenetre::sendNewPicture);
    connect(ui->bConnect1, &QPushButton::clicked, this, &fenetre::onConnect1);
    connect(ui->bConnect2, &QPushButton::clicked, this, &fenetre::onConnect2);
    connect(&wsSender1, &Sender::stateChanged, this, &fenetre::onWS1StateChanged);
    connect(&wsSender2, &Sender::stateChanged, this, &fenetre::onWS2StateChanged);
    connect(ui->sDelay, &QSpinBox::editingFinished, this, &fenetre::onTimerChange);
    connect(ui->cbLayerDirection, &QComboBox::currentTextChanged, this, &fenetre::onTransformChanged);
    connect(ui->sLayerIndex, SIGNAL(valueChanged(int)), this, SLOT(onTransformChanged()));
    connect(ui->cbLayerRotation, &QComboBox::currentTextChanged, this, &fenetre::onTransformChanged);
    connect(ui->checkMiroirV, &QCheckBox::stateChanged, this, &fenetre::onTransformChanged);
    connect(ui->checkMiroirH, &QCheckBox::stateChanged, this, &fenetre::onTransformChanged);
    timerUpdate.start(ui->sDelay->value());
}

fenetre::~fenetre()
{
    delete ui;
}

QImage fenetre::shootScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    return screen->grabWindow(0).toImage();
}

void fenetre::sendNewPicture()
{
    if(!wsSender1.isConnected()  && !wsSender2.isConnected()) return;

    QImage img;
    img = shootScreen();

    // Apply transformations
    img = img.scaled(XLENGTH, YLENGTH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    if(layerMiroirV || layerMiroirH) {
        img = img.mirrored(layerMiroirV, layerMiroirH);
    }
    if(layerRotation != 0) {
        QTransform transform;
        img.trueMatrix(transform, 2, 2);
        transform.rotate(layerRotation);
        img = img.transformed(transform);
    }

    bool luminositeAuCarre = ui->checkLumAuCarre->isChecked();
    float saturationFactor = ui->sSaturationFactor->value();

    int imgArray[YLENGTH][XLENGTH] = {{0}};
    for(int y = 0; y < YLENGTH; ++y) {
        for(int x = 0; x < XLENGTH; ++x) {
            QColor color = img.pixelColor(x, YLENGTH - 1 - y).toHsv();
            float hue = color.hsvHueF();
            float saturation = color.hsvSaturationF();
            float lightness = color.valueF();
            if (luminositeAuCarre) {
                lightness = lightness * lightness;
                if(lightness > 1) lightness = 1;
            }
            saturation *= saturationFactor;
            if(saturation > 1) saturation = 1;
            imgArray[y][x] = (int)QColor::fromHsvF(hue, saturation, lightness).rgb();
        }
    }
    wsSender1.sendImage(imgArray);
    wsSender2.sendImage(imgArray);
}

QString fenetre::getStringState(int state) const
{
    switch(state) {
    case 0:
        return "Connecter";
    case 1:
        return "Connection...";
    case 2:
        return "Déconnecter";
    case 3:
        return "Déconnexion...";
    default:
        return "Inconnu:" + QString::number(state);
    }
}

void fenetre::onConnect1()
{
    if(wsSender1.isConnected()) {
        wsSender1.end();
    }
    else {
        wsSender1.start(ui->lAddresse1->text(), ui->lPassword1->text());
    }
}

void fenetre::onConnect2()
{
    if(wsSender2.isConnected()) {
        wsSender2.end();
    }
    else {
        wsSender2.start(ui->lAddresse2->text(), ui->lPassword2->text());
    }
}

void fenetre::onWS1StateChanged(int state)
{
    ui->bConnect1->setText(getStringState(state));
}

void fenetre::onWS2StateChanged(int state)
{
    ui->bConnect2->setText(getStringState(state));
}

void fenetre::onTimerChange()
{
    timerUpdate.setInterval(ui->sDelay->value());
}
void fenetre::onTransformChanged()
{
    QString layerDirection = ui->cbLayerDirection->currentText();
    wsSender1.setLayerDirection(layerDirection);
    wsSender2.setLayerDirection(layerDirection);

    int layerIndex = ui->sLayerIndex->value();
    wsSender1.setLayerIndex(layerIndex);
    wsSender2.setLayerIndex(layerIndex);

    layerRotation = ui->cbLayerRotation->currentText().toInt();
    layerMiroirV = ui->checkMiroirV->isChecked();
    layerMiroirH = ui->checkMiroirH->isChecked();
}
