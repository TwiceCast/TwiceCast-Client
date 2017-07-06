#ifndef USER_HPP
#define USER_HPP

#include <QString>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>

class User
{
public:
    User(const QString &token = "");
    ~User(void);

public:
    QString getToken(void) const;
    QString getUsername(void) const;
    QString getID(void) const;

public:
    void setToken(const QString &);
    void setUsername(const QString &);
    void setID(const QString &);

private:
    QString m_token;
    QString m_username;
    QString m_id;
};

#endif // USER_HPP
