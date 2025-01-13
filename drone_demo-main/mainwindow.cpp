#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
   // ui->actionTriangles->setChecked(ui->widget->showTriangles);

    /* preset initial positions of the drones */
    const QVector<Vector2D> tabPos={{60,80},{400,700},{50,250},{800,800},{700,50}};

    int n=0;
    for (auto &pos:tabPos) {
        QListWidgetItem *LWitems=new QListWidgetItem(ui->listDronesInfo);
        ui->listDronesInfo->addItem(LWitems);
        QString name="Drone"+QString::number(++n);
        mapDrones[name]=new Drone(name);
        mapDrones[name]->setInitialPosition(pos);
        ui->listDronesInfo->setItemWidget(LWitems,mapDrones[name]);
    }

    ui->widget->setMap(&mapDrones);
    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    timer->start();

    elapsedTimer.start();
}


MainWindow::~MainWindow() {
    delete ui;
    delete timer;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

/*void MainWindow::on_actionTriangles_triggered(bool checked) {


  ui->widget->showTriangles = checked;
    update();
}
*/


void MainWindow::on_actionLoad_triggered() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open JSON File", "", "JSON Files (*.json)");
    if (filePath.isEmpty()) return;

    // Clear existing servers and drones
    for (Server *server : servers) {
        delete server; // Free memory
    }
    servers.clear();

    for (auto &drone : mapDrones) {
        delete drone; // Free memory for drones
    }
    mapDrones.clear();
    ui->listDronesInfo->clear(); // Clear the UI list of drones

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open JSON file!");
        return;
    }

    QByteArray fileData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);

    if (!jsonDoc.isObject()) {
        QMessageBox::warning(this, "Error", "Invalid JSON format!");
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // Parse servers
    QJsonArray serversArray = jsonObj["servers"].toArray();
    for (const QJsonValue &serverVal : serversArray) {
        QJsonObject serverObj = serverVal.toObject();
        QString name = serverObj["name"].toString();
        QString positionStr = serverObj["position"].toString();
        QString color = serverObj["color"].toString();
        QStringList posList = positionStr.split(",");
        if (posList.size() == 2) {
            Vector2D position(posList[0].toDouble(), posList[1].toDouble());
            servers.append(new Server(name, position, color)); // Add server to list
        }
    }

    for (int i = 0; i < servers.size(); ++i) {
        for (int j = i + 1; j < servers.size(); ++j) {
            for (int k = j + 1; k < servers.size(); ++k) {
                Vector2D v0 = servers[i]->getPosition();
                Vector2D v1 = servers[j]->getPosition();
                Vector2D v2 = servers[k]->getPosition();

                QString triColorStr = servers[i]->getColor(); // You might want to adjust this logic
                QColor triColor(triColorStr.isEmpty() ? "#FFFF00" : triColorStr);

                ui->widget->addTriangle(v0, v1, v2, triColor);
            }
        }
    }
     ui->widget->flippAll();

    ui->widget->setServers(servers); // Pass to Canvas
    ui->widget->update(); // Trigger a repaint if needed

    // Parse drones
    QJsonArray dronesArray = jsonObj["drones"].toArray();
    for (const QJsonValue &droneVal : dronesArray) {
        QJsonObject droneObj = droneVal.toObject();
        QString name = droneObj["name"].toString();
        QString positionStr = droneObj["position"].toString();

        QStringList posList = positionStr.split(",");
        if (posList.size() == 2) {
            Vector2D position(posList[0].toDouble(), posList[1].toDouble());
            mapDrones[name] = new Drone(name);
            mapDrones[name]->setInitialPosition(position);

            // Add drone to the list UI
            QListWidgetItem *LWitems = new QListWidgetItem(ui->listDronesInfo);
            ui->listDronesInfo->addItem(LWitems);
            ui->listDronesInfo->setItemWidget(LWitems, mapDrones[name]);
        }
    }

    ui->widget->setMap(&mapDrones);
    repaint();
}








void MainWindow::update() {
    static int last=elapsedTimer.elapsed();
    static int steps=5;
    int current=elapsedTimer.elapsed();
    double dt=(current-last)/(1000.0*steps);
    for (int step=0; step<steps; step++) {
        // update positions of drones
        for (auto &drone:mapDrones) {
            // detect collisions between drone and other flying drones
            if (drone->getStatus()!=Drone::landed) {
                drone->initCollision();
                for (auto &obs:mapDrones) {
                    if (obs->getStatus()!=Drone::landed && obs->getName()!=drone->getName()) {
                        Vector2D B=obs->getPosition();
                        drone->addCollision(B,ui->widget->droneCollisionDistance);
                    }
                }
            }
            drone->update(dt);
        }
    }
    int d = elapsedTimer.elapsed()-current;
    ui->statusbar->showMessage("duree:"+QString::number(d)+" steps="+QString::number(steps));
    if (d>90) {
        steps/=2;
    } else {

        if (steps<10) steps++;
    }
    last=current;
    ui->widget->repaint();
}


