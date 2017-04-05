#ifndef USER_HPP
#define USER_HPP

#include <QString>

class User
{
public:
    User(void);

public:
    QString getToken(void) const;
    QString getUsername(void) const;

public:
    void setToken(const QString &);
    void setUsername(const QString &);

private:
    QString m_token;
    QString m_username;
};

#endif // USER_HPP
