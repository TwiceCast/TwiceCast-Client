#include "User.hpp"

User::User(void)
{

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

void User::setToken(const QString &token)
{
    this->m_token = token;
}

void User::setUsername(const QString &username)
{
    this->m_username = username;
}
