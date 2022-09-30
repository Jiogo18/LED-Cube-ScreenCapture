#include "Sender.h"

// 1 frame, 10 ms, black
static const QString emptyAnimation = "2 0eNqrVipLLSrOzM9TsjLSUUrOzytJzSsJqSxIVbJSSszLzE0sAcnpKKUVJeamOueX5pUoWRlCuS6lRRBpK0MDHaU8oAhQk19quQKGxmIlq2ggMNBBgbE6AyoCFBp10aiLRl00Ml0UG1sLALwIERk=";

Sender::Sender()
{
    connect(&WS, &QWebSocket::connected, this, &Sender::onConnected);
    connect(&WS, &QWebSocket::disconnected, this, &Sender::onDisconnected);
    connect(&WS, &QWebSocket::textMessageReceived, this, &Sender::onTextMessageReceived);
    state = 0;
}

void Sender::sendEmptyAnimation()
{
    if(!isConnected()) return;
    QJsonObject json;
    json.insert("action", "animation.play-no-save");
    json.insert("animation", emptyAnimation);
    sendMessage("ledcube", json);
}

void Sender::sendImage(int img[YLENGTH][XLENGTH])
{
    if(!isConnected()) return;

    QJsonArray layer;
    for(int y = 0; y < YLENGTH; ++y) {
        QJsonArray row;
        for(int x = 0; x < XLENGTH; ++x) {
            row.append(img[y][x]);
        }
        layer.append(row);
    }

    QJsonObject json;
    json.insert("action", "layer.set");
    json.insert("version", 2);
    json.insert("contentType", "layer");
    json.insert("layerIndex", layerIndex);
    json.insert("direction", layerDirection);
    json.insert("lines", layer);
    sendMessage("ledcube", json);
}

void Sender::start(QString url, QString password)
{
    this->password = password;
    emit stateChanged(1);
    state = 1;
    WS.open(QUrl(url));
}

void Sender::end()
{
    switch(state) {
    case 0:
    case 1:
        break;
    case 2:
        emit stateChanged(3);
        state = 3;
    case 3:
        break;
    default:
        break;
    }
    WS.abort();
}

void Sender::onConnected()
{
    qDebug() << "Connected, waiting for server challenge";
}

void Sender::onDisconnected()
{
    qDebug() << "Disconnected, reason:" << WS.closeReason();
    emit stateChanged(0);
    state = 0;
}

void Sender::onTextMessageReceived(QString message)
{
    QJsonObject json = QJsonDocument::fromJson(message.toUtf8()).object();
    QString type = json.value("type").toString();

    if((type != "answer" || !json.value("success").toBool()) && type != "clientCount") {
        qDebug() << "Message received: " << message;
    }

    if(type == "connection") {
        QString status = json.value("status").toString();
        if(status.startsWith("login.failed")) {
            qDebug() << "Login failed: " << status << "=> disconnecting";
            end();
        }
        else if(status.startsWith("login.success")) {
            emit stateChanged(2);
            state = 2;
            sendEmptyAnimation();
        }
    }
    else if(type == "auth") {
        QString content = json.value("content").toString();
        if(content == "challenge") {
            std::string serverChallenge = json.value("challenge").toString().toStdString();
            if(serverChallenge == "") {
                qWarning() << "Auth Challenge received, but no server challenge";
            }

            QJsonObject json;
            std::string clientChallenge = gensalt();
            std::string cookieKey = hash(password.toStdString(), clientChallenge);
            std::string key = bcrypt::generateHash(cookieKey + serverChallenge);
            json.insert("key", key.c_str());
            json.insert("challenge", clientChallenge.c_str());
            sendMessage("auth", json);
            qDebug() << "Sending password";
        }
    }else if(type == "answer") {
        QString msg = json.value("msg").toString();
        if(msg == "You are not authentified.") {
            qDebug() << "Answer: " << msg << "=> disconnecting";
            end();
        }
    }

}

void Sender::sendMessage(QString type, QJsonObject json)
{
    if(!WS.isValid()) return;
    if(type != "auth" && state != 2) return;

    json.insert("type", type);
    json.insert("date", QDateTime::currentDateTime().toString());
    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    if(WS.isValid()) WS.sendTextMessage(strJson);
}

#include "Bcrypt.cpp/src/node_blf.h"
#include "Bcrypt.cpp/src/openbsd.h"
void bcrypt_gensalt(char minor, u_int8_t log_rounds, u_int8_t *seed, char *gsalt);
std::string Sender::gensalt() const {
  char salt[_SALT_LEN];

  unsigned char seed[17]{};
  arc4random_buf(seed, 16);

  bcrypt_gensalt('b', 4, seed, salt);
  return salt;
}

void node_bcrypt(const char *key, size_t key_len, const char *salt, char *encrypted);
std::string Sender::hash(std::string key, std::string salt) const
{
    std::string hash(61, '\0');
    node_bcrypt(key.c_str(), key.size(), salt.c_str(), &hash[0]);
    hash.resize(60);
    return hash;
}
