#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QFileDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void directoryWatchedChanged(const QString &);
    void fileWatchedChanged(const QString &);

    void on_buttonChangeWatchedDirectory_clicked();

private:
    Ui::MainWindow *m_ui;
    QFileSystemWatcher m_watch;
    QString m_pathWatched;
};

#endif // MAINWINDOW_HPP
