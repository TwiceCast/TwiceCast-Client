#include <QTextCodec>
#include "MainWindow.hpp"
#include "NetworkManager.hpp"

NetworkManager::NetworkManager(MainWindow *main, QObject *parent) :
    QObject(parent),
    m_main(main),
    m_baseUrlApi("https://api.twicecast.ovh"),
    m_strike(0)
{
}

NetworkManager::~NetworkManager(void)
{
    delete (this->m_network);
    delete (this->m_timer);
    delete (this->m_ws);
}

void NetworkManager::init()
{
    this->m_network = new QNetworkAccessManager;
    this->m_ws = new QWebSocket;
    this->m_timer = new QTimer;

    connect(m_main, SIGNAL(toggleWs(bool, QString)), this, SLOT(toggleConnection(bool, QString)));
    connect(m_main, SIGNAL(sendText(QString)), this, SLOT(sendData(QString)));
    connect(m_main, SIGNAL(sendWriteFile(QFile*,QString)), this, SLOT(sendWriteFile(QFile*,QString)));
    connect(m_main, SIGNAL(sendRemoveFile(QString)), this, SLOT(sendRemoveFile(QString)));
    connect(m_main, SIGNAL(request(QNetworkAccessManager::Operation,QString,QStringList,QStringList)),
            this, SLOT(request(QNetworkAccessManager::Operation,QString,QStringList,QStringList)));
    connect(this->m_network, SIGNAL(finished(QNetworkReply*)), this, SIGNAL(responseReady(QNetworkReply*)));
    connect(this->m_ws, SIGNAL(textMessageReceived(QString)), this, SLOT(messageReceived(const QString &)));
    connect(this->m_ws, SIGNAL(pong(quint64,QByteArray)), this, SLOT(pongReceived()));
    connect(this->m_ws, SIGNAL(connected()), this, SIGNAL(wsConnection()));
    connect(this->m_ws, static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
        [=](QAbstractSocket::SocketError error){ emit wsConnection(false); emit wsError(error, this->m_ws->errorString()); });
    connect(this->m_timer, SIGNAL(timeout()), this, SLOT(pingSending()));
}

QNetworkReply *NetworkManager::headRequest(const QString &url, const QStringList &, const QStringList &headers) const
{
    QNetworkRequest request;

    for (auto header : headers)
        request.setRawHeader(header.mid(0, header.indexOf('=')).toUtf8(), header.mid(header.indexOf('=') + 1).toUtf8());
    request.setUrl(this->m_baseUrlApi + url);
    return (this->m_network->head(request));
}

QNetworkReply *NetworkManager::putRequest(const QString &url, const QStringList &parameters, const QStringList &headers) const
{
    QNetworkRequest request;
    QString params;

    for (auto header : headers)
        request.setRawHeader(header.mid(0, header.indexOf('=')).toUtf8(), header.mid(header.indexOf('=') + 1).toUtf8());
    for (auto parameter : parameters)
        params += parameter + "\n";
    request.setUrl(this->m_baseUrlApi + url);
    return (this->m_network->put(request, params.toUtf8()));
}

QNetworkReply *NetworkManager::postRequest(const QString &url, const QStringList &parameters, const QStringList &headers) const
{
    QNetworkRequest request;
    QJsonDocument doc;
    QJsonObject object;

    for (auto header : headers)
        request.setRawHeader(header.mid(0, header.indexOf('=')).toUtf8(), header.mid(header.indexOf('=') + 1).toUtf8());
    for (auto parameter : parameters)
        object.insert(parameter.mid(0, parameter.indexOf('=')), parameter.mid(parameter.indexOf('=') + 1));
    request.setUrl(this->m_baseUrlApi + url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    doc.setObject(object);
    return (this->m_network->post(request, doc.toJson()));
}

QNetworkReply *NetworkManager::getRequest(const QString &url, const QStringList &parameters, const QStringList &headers) const
{
    QNetworkRequest request;
    QString params;

    for (auto header : headers)
        request.setRawHeader(header.mid(0, header.indexOf('=')).toUtf8(), header.mid(header.indexOf('=') + 1).toUtf8());
    for (int i = 0; i < parameters.size(); i++)
        params += (i == 0 ? "?" : "&") + parameters[i];
    request.setUrl(this->m_baseUrlApi + url + params);
    return (this->m_network->get(request));
}

QNetworkReply *NetworkManager::deleteRequest(const QString &url, const QStringList &, const QStringList &headers) const
{
    QNetworkRequest request;

    for (auto header : headers)
        request.setRawHeader(header.mid(0, header.indexOf('=')).toUtf8(), header.mid(header.indexOf('=') + 1).toUtf8());
    request.setUrl(this->m_baseUrlApi + url);
    return (this->m_network->deleteResource(request));
}

QNetworkReply *NetworkManager::customRequest(const QString &url, const QStringList &parameters, const QStringList &headers) const
{
    QNetworkRequest request;

    for (auto header : headers)
        request.setRawHeader(header.mid(0, header.indexOf('=')).toUtf8(), header.mid(header.indexOf('=') + 1).toUtf8());
    if (parameters.length() < 1)
        return (NULL);
    request.setUrl(this->m_baseUrlApi + url);
#if QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 8
    QString params;

    for (int i = 1; i < parameters.length(); i++)
        params += parameters[i] + "\n";
    return (this->m_network->sendCustomRequest(request, parameters[0].toUtf8(), params.toUtf8()));
#else
    return (this->m_network->sendCustomRequest(request, parameters[0].toUtf8()));
#endif
}

void NetworkManager::request(QNetworkAccessManager::Operation op, const QString &url, const QStringList &parameters, const QStringList &headers)
{
    static RequestFunc tab[REQUEST_NUMBER] = {
        {QNetworkAccessManager::HeadOperation, &NetworkManager::headRequest},
        {QNetworkAccessManager::PutOperation, &NetworkManager::putRequest},
        {QNetworkAccessManager::PostOperation, &NetworkManager::postRequest},
        {QNetworkAccessManager::GetOperation, &NetworkManager::getRequest},
        {QNetworkAccessManager::DeleteOperation, &NetworkManager::deleteRequest},
        {QNetworkAccessManager::CustomOperation, &NetworkManager::customRequest}
    };

    if (this->m_network->networkAccessible() == QNetworkAccessManager::NotAccessible) {
        emit requestError(NetworkResponse::NETWORK_NOT_CONNECTED, "Network is not accessible");
        return;
    }
    for (int i = 0; i < REQUEST_NUMBER; i++) {
        if (tab[i].op == op)
            (this->*tab[i].func)(url, parameters, headers);
    }
}

qint64 NetworkManager::sendBinaryMessage(const QByteArray &msg) const
{
    if (this->m_ws->isValid())
        return (this->m_ws->sendBinaryMessage(msg));
    return (0);
}

qint64 NetworkManager::sendTextMessage(const QString &msg) const
{
    if (this->m_ws->isValid())
        return (this->m_ws->sendTextMessage(msg));
    return (0);
}

bool NetworkManager::isConnected(void) const
{
    return (this->m_ws->isValid());
}

void NetworkManager::connectWs(const QString &url)
{
    QNetworkRequest request;

    request.setUrl((url == "" ? "ws://localhost:3005" : url));
    this->m_ws->open(request);
    this->m_timer->start(10000);
    this->m_strike = 0;
    this->m_filesParts = new QMap<QString, QString *>();
}

void NetworkManager::disconnectWs(void)
{
    this->m_timer->stop();
    this->m_ws->close();
    emit wsConnection(false);
}

void NetworkManager::pingSending(void)
{
    if (!this->m_ws->isValid())
        return;
    this->m_ws->ping();
    this->m_strike++;
    if (this->m_strike >= 3) {
        this->m_timer->stop();
        this->m_ws->close(QWebSocketProtocol::CloseCodeGoingAway, "No pong received for three times");
        emit wsConnection(false);
        emit wsError(QAbstractSocket::NetworkError, "Server protocol seems to not match client protocol");
    }
}

void NetworkManager::pongReceived(void)
{
    this->m_strike = 0;
}

void NetworkManager::authenticated(const QJsonDocument &document)
{
    emit wsAuthenticated(document.object()["data"].toString());
}

void NetworkManager::fileSent(const QJsonDocument &document)
{
    emit wsFileSent(document.object()["data"].toObject()["file"].toString());
}

void NetworkManager::filePartReceived(const QJsonDocument &document)
{
    QJsonObject data = document.object()["data"].toObject();

    if (!this->m_filesParts->contains(data["name"].toString())) {
        this->m_filesParts->insert(data["name"].toString(), new QString[data["maxPart"].toInt()]);
        for (int i = 0; i < data["maxPart"].toInt(); i++)
            this->m_filesParts->value(data["name"].toString())[i] = "";
    }
    this->m_filesParts->value(data["name"].toString())[data["part"].toInt() - 1] = data["content"].toString();
    QString final = "";
    for (int i = 0; i < data["maxPart"].toInt(); i++) {
        if (this->m_filesParts->value(data["name"].toString())[i] == "")
            return;
        final += this->m_filesParts->value(data["name"].toString())[i];
    }
    qDebug().noquote() << QString::fromUtf8(QByteArray::fromBase64(final.toUtf8()));
    emit prFileReceived(data["name"].toString(), final.toUtf8());
}

void NetworkManager::fileDeleted(const QJsonDocument &document)
{
    emit wsFileDeleted(document.object()["data"].toObject()["file"].toString());
}

void NetworkManager::pullRequestCreated(const QJsonDocument &document)
{
    QJsonDocument data;

    data.setObject(document.object()["data"].toObject());
    emit newPullRequest(data);
}

void NetworkManager::authenticationError(const QJsonDocument &document)
{
    emit wsFailAuth(document.object()["message"].toString());
}

void NetworkManager::writeFileError(const QJsonDocument &document)
{
    emit wsFailWrite(document.object()["file"].toString(), document.object()["message"].toString());
}

void NetworkManager::readFileError(const QJsonDocument &document)
{
    qDebug() << document;
}

void NetworkManager::pullRequestEnded(const QJsonDocument &)
{
    this->m_filesParts->clear();
    emit prFinished();
}

void NetworkManager::messageReceived(const QString &msg)
{
    static WebsocketFunc tab[WEBSOCKET_NUMBER] = {
        {401, "badAuthError", &NetworkManager::authenticationError},
        {409, "failWriteFileError", &NetworkManager::writeFileError},
        {409, "failReadFileError", &NetworkManager::readFileError},
        {409, "failReadPathError", &NetworkManager::readFileError},
        {200, "authentication", &NetworkManager::authenticated},
        {200, "fileAuth", &NetworkManager::authenticated},
        {200, "filePost", &NetworkManager::fileSent},
        {200, "fileDeleted", &NetworkManager::fileDeleted},
        {200, "pullRequestCreated", &NetworkManager::pullRequestCreated},
        {200, "fileGet", &NetworkManager::filePartReceived},
        {200, "pullRequestGet", &NetworkManager::pullRequestEnded}
    };
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
    QJsonObject object;

    qDebug().noquote() << msg;
    if (doc.isNull())
        return;
    object = doc.object();
    for (int i = 0; i < WEBSOCKET_NUMBER; i++)
        if (object["code"].toInt() == tab[i].code && object["type"].toString() == tab[i].type)
            (this->*tab[i].func)(doc);
}

void NetworkManager::sendRemoveFile(const QString &remove)
{
    QJsonDocument document;
    QJsonObject object, data;

    if (!this->isConnected())
        return;
    object.insert("type", "file");
    object.insert("subtype", "delete");
    data.insert("name", remove);
    object.insert("data", data);
    document.setObject(object);
    this->sendTextMessage(QString::fromUtf8(document.toJson()));
}

void NetworkManager::sendWriteFile(QFile *file, const QString &name)
{
    QJsonDocument document;
    QJsonObject object, data;
    QString content;

    if (!this->isConnected() || !file->open(QFile::ReadOnly))
        return;
    content = QString::fromUtf8(file->readAll().toBase64());
    file->close();
    object.insert("type", "file");
    object.insert("subtype", "post");
    data.insert("name", name);
    if (content.length() <= 500) {
        data.insert("content", content);
        object.insert("data", data);
        document.setObject(object);
        this->sendBinaryMessage(document.toJson(QJsonDocument::Indented));
        emit filePartSent(name, 1, 1);
    }
    else
        for (int i = 0; i < content.length(); i += 500) {
            data.insert("content", content.mid(i, 500));
            data.insert("part", (i / 500) + 1);
            data.insert("maxPart", (content.length() / 500) + 1);
            object.insert("data", data);
            document.setObject(object);
            this->sendBinaryMessage(document.toJson(QJsonDocument::Indented));
            qDebug() << "Sending part " << i / 500 + 1 << " from file " << name;
            emit filePartSent(name, (i / 500) + 1, (content.length() / 500) + 1);
        }
}

void NetworkManager::sendData(const QString &text)
{
    if (this->isConnected())
        this->sendTextMessage(text);
}

void NetworkManager::toggleConnection(bool connect, const QString &url)
{
    if (!connect && this->m_ws->isValid())
        this->disconnectWs();
    else if (connect && !this->m_ws->isValid())
        this->connectWs(url);
}
