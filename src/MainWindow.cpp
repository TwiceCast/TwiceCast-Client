#include <QDebug>
#include "MainWindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_pathWatched("")
{
    this->m_ui->setupUi(this);
    connect(&this->m_watch, SIGNAL(directoryChanged(QString)), this, SLOT(directoryWatchedChanged(QString)));
    connect(&this->m_watch, SIGNAL(fileChanged(QString)), this, SLOT(fileWatchedChanged(QString)));
}

MainWindow::~MainWindow(void)
{
    delete (this->m_ui);
}

void MainWindow::directoryWatchedChanged(const QString &path)
{
    qDebug() << "Directory " << path << " has changed";
    //Watch for file name changed (?)
}

void MainWindow::fileWatchedChanged(const QString &path)
{
    qDebug() << "File " << path << " has changed";
}

void MainWindow::on_buttonChangeWatchedDirectory_clicked(void)
{
    this->m_watch.removePath(this->m_pathWatched);
    this->m_pathWatched = QFileDialog::getExistingDirectory(this,
                          "Watched Directory",
                          this->m_pathWatched);
    this->m_ui->watchedDirectory->setText((this->m_pathWatched == "" ? "None" : this->m_pathWatched));
    this->m_watch.addPath(this->m_pathWatched);
    //Watch path recursively
}
