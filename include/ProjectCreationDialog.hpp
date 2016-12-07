#ifndef PROJECTCREATIONDIALOG_HPP
#define PROJECTCREATIONDIALOG_HPP

#include <QFileDialog>
#include <QDialog>
#include <QMap>

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

public:
    QMap<ProjectCreationDialog::ProjectSettings, QString> getValues() const;

private:
    Ui::ProjectCreationDialog *m_ui;
};

#endif // PROJECTCREATIONDIALOG_HPP
