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

    /* preset initial positions of the drones */
    const QVector<Vector2D> tabPos = {{60,80}, {400,700}, {50,250}, {800,800}, {700,50}};

    // Create 5 drones initially
    int n=0;
    for (auto &pos : tabPos) {
        QListWidgetItem *LWitems = new QListWidgetItem(ui->listDronesInfo);
        ui->listDronesInfo->addItem(LWitems);
        QString name = "Drone" + QString::number(++n);

        mapDrones[name] = new Drone(name);
        mapDrones[name]->setInitialPosition(pos);

        ui->listDronesInfo->setItemWidget(LWitems, mapDrones[name]);
    }

    // Let the canvas know about our drones
    ui->widget->setMap(&mapDrones);

    // Setup a timer to update drones
    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start();

    // Start our elapsed timer
    elapsedTimer.start();
}

MainWindow::~MainWindow() {
    delete ui;
    delete timer;
    delete voronoi;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionLoad_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open JSON File", "", "JSON Files (*.json)");
    if (filePath.isEmpty()) return;

    // Clear existing servers
    for (Server *server : servers) {
        delete server;
    }
    servers.clear();

    // Clear existing drones
    for (auto &drone : mapDrones) {
        delete drone;
    }
    mapDrones.clear();
    ui->listDronesInfo->clear(); // Clear the UI list of drones

    // Open JSON
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

    // Parse servers from JSON
    QJsonArray serversArray = jsonObj["servers"].toArray();
    MyPolygon polygon(serversArray.size()); // Assuming maximum of one vertex per server

    for (const QJsonValue &serverVal : serversArray) {
        QJsonObject serverObj = serverVal.toObject();
        QString name = serverObj["name"].toString();
        QString positionStr = serverObj["position"].toString();
        QStringList posList = positionStr.split(",");
        if (posList.size() == 2) {
            float x = posList[0].toDouble();
            float y = posList[1].toDouble();
            Vector2D pos(x, y);
            allPoints.append(pos);
            servers.append(new Server(name, pos, serverObj["color"].toString()));
        }
    }
    ui->widget->clear();


    // Compute convex hull
    for (const Vector2D& point : allPoints) {
        polygon.addVertex(point.x, point.y);
    }
    polygon.computeConvexHull();

    // Clear the canvas

    // The points in polygon.tabPts are now the convex hull points
    // Add non-hull points as interior points
    QVector<Vector2D> hullPoints = polygon.getHullVertices();

    for (const Vector2D& point : allPoints) {
        bool isInterior = true;
        for (const Vector2D& hullPoint : hullPoints) {
            if (point == hullPoint) {
                isInterior = false;
                break;
            }
        }
        if (isInterior) {
            polygon.addInteriorPoint(point);
        }
    }
    qDebug()<<"Calling flip itt33333";

    // Perform ear-clipping triangulation
    polygon.earClippingTriangulate();
    polygon.integrateInteriorPoints(); // Integrate interior points
   //ui->widget->flippAll();
     ui->widget->setPolygon(polygon);
     ui->widget->setServers(servers);

    // Generate Voronoi cells and repaint
  //  ui->widget->generateVoronoiCells(); // Ensure this method exists in Canvas and updates its internal state
    ui->widget->update();

    // Parse drones from JSON
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

            // Add drone to the UI list
            QListWidgetItem *LWitems = new QListWidgetItem(ui->listDronesInfo);
            ui->listDronesInfo->addItem(LWitems);
            ui->listDronesInfo->setItemWidget(LWitems, mapDrones[name]);
        }
    }

    ui->widget->setMap(&mapDrones);
    distributeDronesEqually();

    // Force a repaint
    repaint();
}


void MainWindow::update()
{
    static int last = elapsedTimer.elapsed();
    static int steps = 5;

    int current = elapsedTimer.elapsed();
    double dt = (current - last) / (1000.0 * steps);

    // Sub-steps
    for (int step = 0; step < steps; step++) {
        // update each drone
        for (auto &drone : mapDrones) {
            if (drone->getStatus() != Drone::landed) {
                drone->initCollision();
                // collision detection with other drones
                for (auto &obs : mapDrones) {
                    if (obs->getStatus() != Drone::landed &&
                        obs->getName() != drone->getName()) {
                        Vector2D B = obs->getPosition();
                        drone->addCollision(B, ui->widget->droneCollisionDistance);
                    }
                }
            }
            drone->update(dt);
        }
    }

    int d = elapsedTimer.elapsed() - current;
    ui->statusbar->showMessage("Duration: " + QString::number(d) + " ms, Steps: " + QString::number(steps));
    if (d > 90) {
        steps /= 2;
    } else {
        if (steps < 10) steps++;
    }
    last = current;

    ui->widget->repaint();
}

// --- New toggles for Show Centers / Show Delaunay ---

void MainWindow::on_actionshowCenters_triggered(bool checked)
{
    // Toggle the boolean in the Canvas
    ui->widget->showCenters = checked;
    ui->widget->flippAll();

    ui->widget->update();
}
void MainWindow::on_actionshowCircles_triggered(bool checked)
{
ui->widget->showCircles=checked;
    update();
}
void MainWindow::on_actionshowDelaunay_triggered(bool checked)
{

    // Toggle the boolean in the Canvas
   ui->widget->checkDelaunay();


    ui->widget->update();
}
void MainWindow::distributeDronesEqually() {
    int serverIndex = 0;
    const int serverCount = servers.size();  // Assuming 'servers' is a QVector<Server*>

    QMapIterator<QString, Drone*> i(mapDrones);
    while (i.hasNext()) {
        i.next();
        Drone* drone = i.value();
        // Assuming each server has a 'name' property
        drone->setServerName(servers[serverIndex]->getName());
        servers[serverIndex]->addDrone(drone);
        serverIndex = (serverIndex + 1) % serverCount;  // Round-robin distribution
    }
}

void MainWindow::on_actionshowVoronoi_triggered() {
    Canvas* canvas = ui->widget;

    if (!canvas) {
        qDebug() << "Canvas not found.";
        return;
    }

    // Generate Voronoi diagram for all server points
    canvas->generateVoronoi();
     ui->widget->update();
}

