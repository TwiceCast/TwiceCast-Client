#ifndef PROJECTOPENDIALOG_HPP
#define PROJECTOPENDIALOG_HPP

#include <QFileDialog>
#include <QDialog>

#include "Project.hpp"

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

private slots:
    void on_pathButton_clicked(void);
    void on_pathLine_textChanged(const QString &);

private:
    Ui::ProjectOpenDialog *m_ui;
};

#endif // PROJECTOPENDIALOG_HPP
