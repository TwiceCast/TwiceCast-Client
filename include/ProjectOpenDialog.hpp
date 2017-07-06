#ifndef PROJECTOPENDIALOG_HPP
#define PROJECTOPENDIALOG_HPP

#include <QFileDialog>
#include <QDialog>

#include "Project.hpp"
#include "User.hpp"

namespace Ui {
class ProjectOpenDialog;
}

class ProjectOpenDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectOpenDialog(QWidget *parent = 0);
    ~ProjectOpenDialog(void);

public:
    Project *getProject(void);

public slots:
    void init(User *);

private slots:
    void on_pathButton_clicked(void);
    void on_pathLine_textChanged(const QString &);
    void oldProjectOpen(const QString &);

private:
    Ui::ProjectOpenDialog *m_ui;
    QLabel *m_oldProject[5];
};

#endif // PROJECTOPENDIALOG_HPP
