#ifndef SENDER_H
#define SENDER_H

#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImage>
#include <Bcrypt.cpp/include/bcrypt.h>

#define YLENGTH 8
#define XLENGTH 8

class Sender : public QObject
{
  Q_OBJECT
public:
  Sender();
  void sendEmptyAnimation();
  void sendImage(int img[YLENGTH][XLENGTH]);

  bool isConnected() { return WS.isValid() && state == 2; }
  void start(QString url, QString password);
  void end();

  void setLayerDirection(QString layerDirection) { this->layerDirection = layerDirection; sendEmptyAnimation(); }
  void setLayerIndex(int layerIndex) { this->layerIndex = layerIndex; sendEmptyAnimation(); }

signals:
  void stateChanged(int state);

private Q_SLOTS:
  void onConnected();
  void onDisconnected();
  void onTextMessageReceived(QString message);

  void sendMessage(QString type, QJsonObject json);

private:
  std::string gensalt() const;
  std::string hash(std::string key, std::string salt) const;
  QWebSocket WS;
  QString password;
  int state;
  QString layerDirection = "Y";
  int layerIndex = 0;
};

#endif // SENDER_H
