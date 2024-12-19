/**
 * @brief Drone_demo project
 * @author B.Piranda
 * @date dec. 2024
 **/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QTimer>
#include <QElapsedTimer>
#include <drone.h>
#include <vector2d.h>
#include <server.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionQuit_triggered();
    void update();

    void on_actionLoad_triggered();

private:
    Ui::MainWindow *ui;
    QMap<QString,Drone*> mapDrones;
    QTimer *timer;
    QElapsedTimer elapsedTimer;

     void loadJsonFile(const QString &filePath); // Method to load and parse JSON file
      QVector<Server *> servers; // List of servers

};
#endif // MAINWINDOW_H
