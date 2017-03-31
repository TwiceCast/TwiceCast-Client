#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include <QObject>
#include <QTimer>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#define OP_NUMBER 6

class NetworkManager : public QObject
{
    Q_OBJECT
    struct OpFunc {
        QNetworkAccessManager::Operation op;
        QNetworkReply *(NetworkManager::*func)(const QString &, const QStringList &) const;
    };

public:
    explicit NetworkManager(QObject *parent = 0);
    ~NetworkManager(void);

private:
    QNetworkReply *headRequest(const QString &, const QStringList &) const;
    QNetworkReply *putRequest(const QString &, const QStringList &) const;
    QNetworkReply *postRequest(const QString &, const QStringList &) const;
    QNetworkReply *getRequest(const QString &, const QStringList &) const;
    QNetworkReply *deleteRequest(const QString &, const QStringList &) const;
    QNetworkReply *customRequest(const QString &, const QStringList &) const;

public:
    QNetworkReply *request(const QString &, const QStringList &, QNetworkAccessManager::Operation) const;
    qint64 sendBinaryMessage(const QByteArray &) const;
    qint64 sendTextMessage(const QString &) const;
    bool isConnected(void) const;

public:
    void connectWs(void);
    void disconnectWs(void);

signals:
    void responseReady(QNetworkReply *);
    void wsConnected(void);

private slots:
    void pingSending(void);
    void pongReceived(void);
    void messageReceived(const QString &);

private:
    QNetworkAccessManager *m_network;
    QWebSocket *m_ws;
    QTimer *m_timer;
    QString m_baseUrlApi, m_baseUrlWs;
    int m_strike;
};

#endif // NETWORKMANAGER_HPP
