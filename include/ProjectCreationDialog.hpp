#ifndef PROJECTCREATIONDIALOG_HPP
#define PROJECTCREATIONDIALOG_HPP

#include <QFileDialog>
#include <QDialog>
#include <QMap>

#include "Project.hpp"

namespace Ui {
class ProjectCreationDialog;
}

class ProjectCreationDialog : public QDialog
{
    Q_OBJECT

public:
    enum ProjectSettings {
        TITLE,
        PATH,
        NONE
    };

public:
    explicit ProjectCreationDialog(QWidget *parent = 0);
    ~ProjectCreationDialog();

private slots:
    void on_browseButton_clicked();

    void on_linePath_textChanged(const QString &arg1);

public:
    Project *getProject() const;

private:
    Ui::ProjectCreationDialog *m_ui;
};

#endif // PROJECTCREATIONDIALOG_HPP
