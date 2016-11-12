#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QFileDialog>
#include <QTreeWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QStringList getFileList(const QString &path, bool recursively = false) const;
    void updateTreeView(void) const;
    QTreeWidgetItem *getTreeItem(const QString &) const;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void directoryWatchedChanged(const QString &);
    void fileWatchedChanged(const QString &);
    void on_actionNewProject_triggered();

private:
    Ui::MainWindow *m_ui;
    QFileSystemWatcher m_watch;
    QString m_pathWatched;
};

#endif // MAINWINDOW_HPP
