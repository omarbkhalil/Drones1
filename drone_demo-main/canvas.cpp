#include "canvas.h"
#include "qevent.h"
#include <QPainter>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <iostream>

// Example includes for your classes (adjust if needed)
// #include "triangle.h"
// #include "drone.h"
// #include "server.h"
// #include "vector2d.h"

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    droneImg.load("../../media/drone.png");
    setMouseTracking(true);
}

Canvas::~Canvas()
{
    clear();
}

void Canvas::clear()
{
    triangles.clear();
    vertices.clear();
    servers.clear();
    if (mapDrones) {
        for (auto &drone : *mapDrones) {
            delete drone;
        }
        mapDrones->clear();
    }
}

void Canvas::addPoints(const QVector<Vector2D> &tab) {
    for (const auto &pt : tab) {
        vertices.push_back(Vector2D(pt));
        qDebug() << "Added vertex:" << pt.x << pt.y; // Debug statement
    }
    std::cout << "Total vertices after addition: " << vertices.size() << std::endl;
    reScale();
    update();
}

void Canvas::addTriangle(int id0, int id1, int id2) {
    triangles.push_back(Triangle(&vertices[id0],&vertices[id1],&vertices[id2]));
}

void Canvas::addTriangle(int id0, int id1, int id2,const QColor &color) {
    triangles.push_back(Triangle(&vertices[id0],&vertices[id1],&vertices[id2],color));
}
void Canvas::addTriangle(const Vector2D &v1, const Vector2D &v2, const Vector2D &v3, const QColor &color) {
    triangles.push_back(Triangle(new Vector2D(v1), new Vector2D(v2), new Vector2D(v3), color));
}


void Canvas::generateTriangles() {
    triangles.clear();
    int n = vertices.size();
    if (n < 3) return;

    // Simple fan triangulation from vertex[0]
    for (int i = 1; i < n - 1; ++i) {
        Vector2D *v0 = &vertices[0];
        Vector2D *v1 = &vertices[i];
        Vector2D *v2 = &vertices[i + 1];
        triangles.push_back(Triangle(v0, v1, v2, Qt::red)); // Directly create and add the triangle
    }
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

void Canvas::generateSimpleTriangles() {
    triangles.clear();
    if (vertices.size() < 3) return; // Need at least three vertices

    const Vector2D& central = vertices[0]; // Choosing the first vertex as central
    for (int i = 1; i < vertices.size() - 1; ++i) {
        addTriangle(0, i, i + 1, Qt::red); // Add triangle between central, i, and i+1
    }
    std::cout << "Triangles added: " << triangles.size() << std::endl;
    for (const auto& tri : triangles) {
        auto p0 = tri.getVertexPtr(0);
        auto p1 = tri.getVertexPtr(1);
        auto p2 = tri.getVertexPtr(2);
        std::cout << "Triangle vertices: (" << p0->x << ", " << p0->y << ") (" << p1->x << ", " << p1->y << ") (" << p2->x << ", " << p2->y << ")\n";
    }

}

void Canvas::flippAll() {
    while (!checkDelaunay()) {
        auto it = triangles.begin();
        while (it != triangles.end() && !it->isFlippable()) {
            it++;
        }
        if (it != triangles.end()) {
            it->flippIt(triangles);
        } else {
            qDebug() << "issue";
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

    // Fill the background
    painter.fillRect(rect(), Qt::white);




    // ----- APPLY THE TRANSFORM (IMPORTANT!) -----
    // This will ensure large or small coordinates
    // are mapped into the visible area of the widget.
    painter.translate(10, 10);
    painter.scale(scaleFactor, scaleFactor);
    painter.translate(-origin.x, -origin.y);

    for (auto &tri : triangles) {
        tri.draw(painter);
    }


    // 2) Draw servers
    QPen serverPen(Qt::blue);
    serverPen.setWidth(5);
    painter.setPen(serverPen);

    for (const Server *server : servers) {
        // Draw server position (ellipse)
        painter.drawEllipse(QPointF(server->getPosition().x,
                                    server->getPosition().y),
                            5, 5);

        // Draw server name near position
        painter.setPen(Qt::black); // text in black
        painter.drawText(QPointF(server->getPosition().x + 10,
                                 server->getPosition().y - 10),
                         server->getName());

        // Optionally restore the pen (blue) for next server
        painter.setPen(serverPen);
    }

    // 3) Draw drones
    if (mapDrones) {
        QPen penCol(Qt::DashDotDotLine);
        penCol.setColor(Qt::lightGray);
        penCol.setWidth(3);

        // Icon rectangles
        QRect rect(-droneIconSize / 2,
                   -droneIconSize / 2,
                   droneIconSize, droneIconSize);

        QRect rectCol(-droneCollisionDistance / 2,
                      -droneCollisionDistance / 2,
                      droneCollisionDistance, droneCollisionDistance);

        for (auto &drone : *mapDrones) {
            painter.save();

            // Translate and rotate to drone’s position & azimuth
            painter.translate(drone->getPosition().x,
                              drone->getPosition().y);
            painter.rotate(drone->getAzimut());
            painter.drawImage(rect, droneImg);

            // Light LEDs if flying
            if (drone->getStatus() != Drone::landed) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(Qt::red);
                painter.drawEllipse((-185.0f / 511.0f) * droneIconSize,
                                    (-185.0f / 511.0f) * droneIconSize,
                                    (65.0f / 511.0f) * droneIconSize,
                                    (65.0f / 511.0f) * droneIconSize);
                painter.drawEllipse((115.0f / 511.0f) * droneIconSize,
                                    (-185.0f / 511.0f) * droneIconSize,
                                    (65.0f / 511.0f) * droneIconSize,
                                    (65.0f / 511.0f) * droneIconSize);
                painter.setBrush(Qt::green);
                painter.drawEllipse((-185.0f / 511.0f) * droneIconSize,
                                    (115.0f / 511.0f) * droneIconSize,
                                    (70.0f / 511.0f) * droneIconSize,
                                    (70.0f / 511.0f) * droneIconSize);
                painter.drawEllipse((115.0f / 511.0f) * droneIconSize,
                                    (115.0f / 511.0f) * droneIconSize,
                                    (70.0f / 511.0f) * droneIconSize,
                                    (70.0f / 511.0f) * droneIconSize);
            }

            // Draw collision detector
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
    if (!event) return; // Safety check

    // Search for a landed drone
    auto it = mapDrones->begin();
    while (it != mapDrones->end() &&
           (*it)->getStatus() != Drone::landed)
    {
        ++it;
    }

    // If found, set the goal position to where user clicked
    if (it != mapDrones->end()) {
        // If you want to map from screen coords to data coords,
        // you might need to reverse the transform (depending on your usage).
        // For now, let's just set it as raw widget coordinates:
        (*it)->setGoalPosition(Vector2D(event->pos().x(),
                                        event->pos().y()));
        (*it)->start();
    }

    update(); // or repaint(), but update() is usually recommended
}

Server* Canvas::findServerByName(const QString& name) {
    for (Server* server : servers) {
        if (server->getName() == name) {
            return server;
        }
    }
    return nullptr; // Return null if no server is found
}

