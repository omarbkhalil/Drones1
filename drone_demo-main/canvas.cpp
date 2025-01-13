#include "canvas.h"
#include "qevent.h"
#include <QPainter>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <iostream>


Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    droneImg.load("../../media/drone.png");
    setMouseTracking(true);
    showTriangles = true; // Ensure triangles are shown by default
    showDelaunay = false; // Delaunay is optional
    showCenters = false;  // Show centers only when toggled
}

Canvas::~Canvas()
{
    clear();
}

void Canvas::clear()
{
    // Clear the triangles and raw vertices
    triangles.clear();
    vertices.clear();

    // Clear polygons
    for (auto poly : polygons) {
        delete poly;
    }
    polygons.clear();

    // Clear servers
    servers.clear();

    // Clear drones if mapDrones
    if (mapDrones) {
        for (auto &drone : *mapDrones) {
            delete drone;
        }
        mapDrones->clear();
    }
}

void Canvas::addPoints(const QVector<Vector2D> &tab)
{
    // Just store them in vertices if you want
    for (const auto &pt : tab) {
        vertices.push_back(pt);
        qDebug() << "Added vertex:" << pt.x << pt.y;
    }
    std::cout << "Total vertices after addition: " << vertices.size() << std::endl;

    reScale();
    update();
}

void Canvas::generateEarClippingTriangles()
{
    // Clear old data
    triangles.clear();
    polygons.clear();

    if (vertices.size() < 3) {
        qDebug() << "Not enough vertices to form a polygon.";
        return;
    }

    // Create a new polygon using the vertices
    MyPolygon *polygon = new MyPolygon(vertices.size());
    for (const auto &vertex : vertices) {
        polygon->addVertex(vertex.x, vertex.y);
    }

    // Ensure the polygon is in counter-clockwise order
    polygon->ensureCCW();

    // Perform ear-clipping triangulation
    polygon->earClippingTriangulate();

    // Store the polygon for rendering
    polygons.push_back(polygon);

    qDebug() << "Ear-clipping triangulation generated.";

    update();
}



bool Canvas::checkDelaunay()
{
    bool areAllDelaunay = true;
    for (auto &triangle : triangles) {
        bool res = triangle.checkDelaunay(vertices);
        if (!res) {
            auto L = findOppositePointOfTriangle(triangle);
            auto it = L.begin();
            while (it != L.end() && triangle.circleContains(*it)) {
                ++it;
            }
            std::cout << "L size: " << L.size() << ", Iterator at end: " << (it != L.end() ? "No" : "Yes") << std::endl;
            triangle.setDelaunay(false, it != L.end());
        }
        areAllDelaunay = res && areAllDelaunay;
    }
    update();
    return areAllDelaunay;
}


QVector<const Vector2D *> Canvas::findOppositePointOfTriangle(Triangle &tri)
{
    QVector<const Vector2D *> list;
    for (auto &otherTri : triangles) {
        if (tri.hasEdge(otherTri.getVertexPtr(1), otherTri.getVertexPtr(0))) {
            list.append(otherTri.getVertexPtr(2));
        }
        else if (tri.hasEdge(otherTri.getVertexPtr(2), otherTri.getVertexPtr(1))) {
            list.append(otherTri.getVertexPtr(0));
        }
        else if (tri.hasEdge(otherTri.getVertexPtr(0), otherTri.getVertexPtr(2))) {
            list.append(otherTri.getVertexPtr(1));
        }
    }
    return list;
}

void Canvas::flippAll()
{
    // keep flipping until checkDelaunay() returns true or no flippable triangles exist
    while (!checkDelaunay()) {
        auto it = triangles.begin();
        while (it != triangles.end() && !it->isFlippable()) {
            it++;
        }
        if (it != triangles.end()) {
            it->flippIt(triangles);
        } else {
            qDebug() << "issue: no more flippable triangles found but still not Delaunay?";
            break;
        }
    }
}

/* void Canvas::loadMesh(const QString &filePath)
{
    std::cout << "loadMesh called with file: " << filePath.toStdString() << std::endl;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cout << "Unable to open file: " << filePath.toStdString() << std::endl;
        return;
    }

    clear();

    QByteArray fileData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
    QJsonObject jsonObj = jsonDoc.object();
    file.close();

    // Parse servers
    QJsonArray serversArray = jsonObj["servers"].toArray();
    QVector<Server*> servers;  // Make sure this is defined somewhere appropriate
int triangleCount = 0;

    for (const QJsonValue &serverVal : serversArray) {
        QJsonObject serverObj = serverVal.toObject();
        QString name = serverObj["name"].toString();
        QString positionStr = serverObj["position"].toString();
        QStringList posList = positionStr.split(",");
        if (posList.size() == 2) {
            Vector2D position(posList[0].toDouble(), posList[1].toDouble());
            servers.append(new Server(name, position, serverObj["color"].toString()));
        }
    }

    // Assuming you have at least three servers and servers.size() % 3 == 0
    for (int i = 0; i + 2 < servers.size(); i += 3) {
        Vector2D v0 = servers[i]->getPosition();
        Vector2D v1 = servers[i+1]->getPosition();
        Vector2D v2 = servers[i+2]->getPosition();

        // Assuming the color of the triangle can be the color of the first server or any logic you prefer
        QString triColorStr = servers[i]->getColor();
        QColor triColor(triColorStr.isEmpty() ? "#FFFF00" : triColorStr);

        // Add triangle to your vector or list
        triangles.append(Triangle(&v0, &v1, &v2, triColor));
        triangleCount++;  // Increment the triangle count
         }


    // Parse drones
    QJsonArray dronesArray = jsonObj["drones"].toArray();
    for (const QJsonValue &droneVal : dronesArray) {
        QJsonObject droneObj = droneVal.toObject();
        QString name = droneObj["name"].toString();
        QString positionStr = droneObj["position"].toString();
        QStringList posList = positionStr.split(",");
        if (posList.size() == 2) {
            Vector2D position(posList[0].toDouble(), posList[1].toDouble());
            (*mapDrones)[name] = new Drone(name);
            (*mapDrones)[name]->setInitialPosition(position);
        }
    }



    // 6) Re-scale and update
    reScale();
    update();
}

*/
void Canvas::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // 1) fill background
    painter.fillRect(rect(), Qt::white);

    // 2) transforms
    painter.translate(10, 10);
    painter.scale(scaleFactor, scaleFactor);
    painter.translate(-origin.x, -origin.y);

    // ----- Draw Polygons -----
    // Each MyPolygon draws its own ear-clipped triangles if showTriangles == true
    for (auto poly : polygons) {
        poly->draw(painter, showTriangles);
    }

    // ----- Draw Centers (if toggled) -----
    if (showCenters) {
        QPen centerPen(Qt::red);
        centerPen.setWidth(4);
        painter.setPen(centerPen);
        painter.setBrush(Qt::NoBrush);

        // Optionally, also for polygons' ear-clipped triangles
        for (auto poly : polygons) {
            const auto &polyTris = poly->getTriangles(); // if getTriangles() is a const accessor
            for (auto &t : polyTris) {
                Vector2D c = t.getCircleCenter();
                painter.drawEllipse(QPointF(c.x, c.y), 3, 3);
            }
        }
    }

    // ----- Draw Servers -----
    QPen serverPen(Qt::blue);
    serverPen.setWidth(5);
    painter.setPen(serverPen);

    for (const Server *server : servers) {
        painter.drawEllipse(QPointF(server->getPosition().x,
                                    server->getPosition().y),
                            5, 5);
        painter.setPen(Qt::black);
        painter.drawText(QPointF(server->getPosition().x + 10,
                                 server->getPosition().y - 10),
                         server->getName());
        painter.setPen(serverPen);
    }

    // ----- Draw Drones -----
    if (mapDrones) {
        QPen penCol(Qt::DashDotDotLine);
        penCol.setColor(Qt::lightGray);
        penCol.setWidth(3);

        QRect rectIcon(-droneIconSize / 2,
                       -droneIconSize / 2,
                       droneIconSize,
                       droneIconSize);

        QRect rectCol(-droneCollisionDistance / 2,
                      -droneCollisionDistance / 2,
                      droneCollisionDistance,
                      droneCollisionDistance);

        for (auto &drone : *mapDrones) {
            painter.save();
            painter.translate(drone->getPosition().x, drone->getPosition().y);
            painter.rotate(drone->getAzimut());
            painter.drawImage(rectIcon, droneImg);

            // collision circle
            if (drone->hasCollision()) {
                painter.setPen(penCol);
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(rectCol);
            }

            painter.restore();
        }
    }
}

void Canvas::resizeEvent(QResizeEvent *)
{
    reScale();
    update();
}

void Canvas::reScale()
{
    if (vertices.isEmpty()) return;

    int newWidth = width() - 20;
    int newHeight = height() - 20;

    auto box = getBox();
    float dataWidth = box.second.x - box.first.x;
    float dataHeight = box.second.y - box.first.y;

    // Use a float member called scaleFactor, not scale()
    scaleFactor = qMin(float(newWidth) / dataWidth,
                       float(newHeight) / dataHeight);

    origin.setX(box.first.x);
    origin.setY(box.first.y);
}

QPair<Vector2D, Vector2D> Canvas::getBox()
{
    if (vertices.isEmpty()) {
        // Default bounding box if no data
        return { Vector2D(0, 0), Vector2D(200, 200) };
    }

    Vector2D infLeft = vertices.first();
    Vector2D supRight = vertices.first();

    for (const auto &v : vertices) {
        if (v.x < infLeft.x)  infLeft.x = v.x;
        if (v.y < infLeft.y)  infLeft.y = v.y;
        if (v.x > supRight.x) supRight.x = v.x;
        if (v.y > supRight.y) supRight.y = v.y;
    }
    return { infLeft, supRight };
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if (!event) return;

    // find a landed drone
    auto it = mapDrones->begin();
    while (it != mapDrones->end() && (*it)->getStatus() != Drone::landed) {
        ++it;
    }

    if (it != mapDrones->end()) {
        // If needed, convert from screen coords to "canvas" coords:
        // But for now, let's just set the clicked pixel as the goal:
        (*it)->setGoalPosition(Vector2D(event->pos().x(), event->pos().y()));
        (*it)->start();
    }
    update();
}

Server* Canvas::findServerByName(const QString &name) {
    for (Server* srv : servers) {
        if (srv->getName() == name) {
            return srv;
        }
    }
    return nullptr;
}
