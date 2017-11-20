#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include <QThread>
#include <QTimer>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTreeWidget>

#include "Stream.hpp"

class MainWindow;

#define REQUEST_NUMBER 6
#define WEBSOCKET_NUMBER 10

namespace NetworkResponse {
    enum NetworkError {
        NETWORK_NOT_CONNECTED,
        UNKNOWN
    };
}

class NetworkManager : public QObject
{
    Q_OBJECT

    struct RequestFunc {
        QNetworkAccessManager::Operation op;
        QNetworkReply *(NetworkManager::*func)(const QString &, const QStringList &, const QStringList &) const;
    };
    struct WebsocketFunc {
        int code;
        QString type;
        void (NetworkManager::*func)(const QJsonDocument &);
    };

public:
    explicit NetworkManager(MainWindow *, QObject *parent = 0);
    ~NetworkManager(void);

private: // HTTP Request functions
    QNetworkReply *headRequest(const QString &, const QStringList &, const QStringList &) const;
    QNetworkReply *putRequest(const QString &, const QStringList &, const QStringList &) const;
    QNetworkReply *postRequest(const QString &, const QStringList &, const QStringList &) const;
    QNetworkReply *getRequest(const QString &, const QStringList &, const QStringList &) const;
    QNetworkReply *deleteRequest(const QString &, const QStringList &, const QStringList &) const;
    QNetworkReply *customRequest(const QString &, const QStringList &, const QStringList &) const;

private: // Response functions for Websocket messages
    void authenticated(const QJsonDocument &);
    void fileSent(const QJsonDocument &);
    void filePartReceived(const QJsonDocument &);
    void fileDeleted(const QJsonDocument &);
    void pullRequestCreated(const QJsonDocument &);
    void authenticationError(const QJsonDocument &);
    void writeFileError(const QJsonDocument &);
    void readFileError(const QJsonDocument &);

private: // Websocket functions
    qint64 sendBinaryMessage(const QByteArray &) const;
    qint64 sendTextMessage(const QString &) const;
    bool isConnected(void) const;
    void connectWs(const QString &);
    void disconnectWs(void);

signals: // Network signals
    void responseReady(QNetworkReply *);
    void requestError(NetworkResponse::NetworkError, const QString &);
    void wsConnection(bool connected = true);
    void wsError(QAbstractSocket::SocketError, const QString &);
    void filePartSent(const QString &, int, int);
    void wsAuthenticated(const QString &);
    void wsFileSent(const QString &);
    void wsFileDeleted(const QString &);
    void newPullRequest(const QJsonDocument &);
    void wsFailAuth(const QString &);
    void wsFailWrite(const QString &, const QString &);

private slots: // Network intern slots
    void pingSending(void);
    void pongReceived(void);
    void messageReceived(const QString &);

public slots: // Network extern slots
    void init(void);
    void request(QNetworkAccessManager::Operation, const QString &, const QStringList &, const QStringList &headers = QStringList());
    void sendRemoveFile(const QString &);
    void sendWriteFile(QFile *, const QString &);
    void sendData(const QString &);
    void toggleConnection(bool, const QString &);

private:
    MainWindow *m_main;
    QNetworkAccessManager *m_network;
    QWebSocket *m_ws;
    QTimer *m_timer;
    QString m_baseUrlApi;
    Stream *m_stream;
    QMap<QString, QString *> *m_filesParts;
    int m_strike;
};

#endif // NETWORKMANAGER_HPP
