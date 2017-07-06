#include <QDebug>
#include "User.hpp"

User::User(const QString &token) :
    m_token(token)
{
    QJsonObject object = QJsonDocument::fromJson(QByteArray::fromBase64(this->m_token.split(".")[1].toUtf8())).object();

    this->setID(object["uid"].toString());
}

User::~User(void)
{

}

QString User::getToken(void) const
{
    return (this->m_token);
}

QString User::getUsername(void) const
{
    return (this->m_username);
}

QString User::getID(void) const
{
    return (this->m_id);
}

void User::setToken(const QString &token)
{
    this->m_token = token;
}

void User::setUsername(const QString &username)
{
    this->m_username = username;
}

void User::setID(const QString &id)
{
    this->m_id = id;
}
