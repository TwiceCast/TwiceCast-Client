#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QTreeWidget>
#include <QMessageBox>
#include <QRegExp>

#include "ProjectCreationDialog.hpp"
#include "IgnoredAdderDialog.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QStringList getFileList(const QString &, bool recursively = false) const;
    QTreeWidgetItem *getTreeItem(const QString &) const;
    QList<QTreeWidgetItem *> getRemovedPaths(QTreeWidgetItem *, const QStringList &) const;
    QList<QString> getAddedPaths(QTreeWidgetItem *, const QStringList &) const;
    QList<QTreeWidgetItem *> findItems(QTreeWidgetItem *, int, const QString &) const;

private:
    void removeByPath(QTreeWidgetItem *, const QString &);
    void checkPath(QTreeWidgetItem *, const QString &, const QStringList &);
    void applyFont(QTreeWidgetItem *);
    void resetFont(QTreeWidgetItem *);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void directoryWatchedChanged(const QString &);
    void fileWatchedChanged(const QString &);
    void addPathIgnored(QTreeWidgetItem *, int);
    void checkTreeIgnored(void);
    void projectCreated(void);
    void addIgnored(void);
    void on_actionExit_triggered(void);

    void on_removeButton_pressed();

private:
    Ui::MainWindow *m_ui;
    QFileSystemWatcher m_watch;
    ProjectCreationDialog m_creation;
    IgnoredAdderDialog m_ignoredAdder;
    QString m_pathWatched;
};

#endif // MAINWINDOW_HPP
