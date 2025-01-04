#include "canvas.h"
#include "qevent.h"
#include <QPainter>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

Canvas::Canvas(QWidget *parent)
    : QWidget{parent} {
    droneImg.load("../../media/drone.png");
    setMouseTracking(true);
}

Canvas::~Canvas() {
    clear();
}

void Canvas::clear() {
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
    for (auto &pt : tab) {
        vertices.push_back(Vector2D(pt));
    }
    generateTriangles();
    reScale();
    update();
}

void Canvas::addTriangle(int id0, int id1, int id2, const QColor &color) {
    triangles.push_back(Triangle(&vertices[id0], &vertices[id1], &vertices[id2], color));
}

void Canvas::generateTriangles() {
    triangles.clear();
    int n = vertices.size();
    if (n < 3) return;

    // Simple triangulation logic (could use Delaunay triangulation algorithm here)
    for (int i = 1; i < n - 1; ++i) {
        addTriangle(0, i, i + 1, Qt::red);
    }
}

QVector<const Vector2D *> Canvas::findOppositePointOfTriangle(Triangle &tri) {
    QVector<const Vector2D *> list;
    for (auto &otherTri : triangles) {
        if (tri.hasEdge(otherTri.getVertexPtr(1), otherTri.getVertexPtr(0))) list.append(otherTri.getVertexPtr(2));
        else if (tri.hasEdge(otherTri.getVertexPtr(2), otherTri.getVertexPtr(1))) list.append(otherTri.getVertexPtr(0));
        else if (tri.hasEdge(otherTri.getVertexPtr(0), otherTri.getVertexPtr(2))) list.append(otherTri.getVertexPtr(1));
    }
    return list;
}

bool Canvas::checkDelaunay() {
    bool areAllDelaunay = true;
    for (auto &triangle : triangles) {
        bool res = triangle.checkDelaunay(vertices);
        if (!res) {
            auto L = findOppositePointOfTriangle(triangle);
            auto it = L.begin();
            while (it != L.end() && triangle.circleContains(*it)) {
                it++;
            }
            qDebug() << L.size() << (it != L.end());
            triangle.setDelaunay(false, it != L.end());
        }
        areAllDelaunay = res && areAllDelaunay;
    }
    update();
    return areAllDelaunay;
}

void Canvas::loadMesh(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Unable to open file";
        return;
    }

    clear();

    QByteArray fileData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);

    QJsonObject jsonObj = jsonDoc.object();

    // Parse servers
    QJsonArray serversArray = jsonObj["servers"].toArray();
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

    // Add all positions to vertices for triangulation
    for (const auto &server : servers) {
        vertices.append(server->getPosition());
    }
    for (const auto &drone : *mapDrones) {
        vertices.append(drone->getPosition());
    }

    generateTriangles();
    reScale();
    update();
}

void Canvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QBrush whiteBrush(Qt::SolidPattern);
    QPen penCol(Qt::DashDotDotLine);
    penCol.setColor(Qt::lightGray);
    penCol.setWidth(3);
    whiteBrush.setColor(Qt::white);
    painter.fillRect(0, 0, width(), height(), whiteBrush);

    // Draw server positions
    QPen serverPen(Qt::blue);
    serverPen.setWidth(5);
    painter.setPen(serverPen);

    for (const Server *server : servers) { // Use Server objects instead of just positions
        // Draw server position
        painter.drawEllipse(QPointF(server->getPosition().x, server->getPosition().y), 5, 5);

        // Draw server name near position
        painter.setPen(Qt::black); // Set black pen for text
        painter.drawText(QPointF(server->getPosition().x + 10, server->getPosition().y - 10), server->getName());
    }

    // Draw drones
    if (mapDrones) {
        QRect rect(-droneIconSize / 2, -droneIconSize / 2, droneIconSize, droneIconSize);
        QRect rectCol(-droneCollisionDistance / 2, -droneCollisionDistance / 2, droneCollisionDistance, droneCollisionDistance);

        for (auto &drone : *mapDrones) {
            painter.save();
            // Place and orient the drone
            painter.translate(drone->getPosition().x, drone->getPosition().y);
            painter.rotate(drone->getAzimut());
            painter.drawImage(rect, droneImg);

            // Light LEDs if flying
            if (drone->getStatus() != Drone::landed) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(Qt::red);
                painter.drawEllipse((-185.0 / 511.0) * droneIconSize, (-185.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize);
                painter.drawEllipse((115.0 / 511.0) * droneIconSize, (-185.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize);
                painter.setBrush(Qt::green);
                painter.drawEllipse((-185.0 / 511.0) * droneIconSize, (115.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize);
                painter.drawEllipse((115.0 / 511.0) * droneIconSize, (115.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize);
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

void Canvas::resizeEvent(QResizeEvent *) {
    reScale();
    update();
}

void Canvas::reScale() {
    int newWidth = width() - 20;
    int newHeight = height() - 20;
    if (vertices.isEmpty()) return;

    auto box = getBox();
    float dataWidth = box.second.x - box.first.x;
    float dataHeight = box.second.y - box.first.y;
    scale = qMin(float(newWidth) / dataWidth, float(newHeight) / dataHeight);
    origin.setX(box.first.x);
    origin.setY(box.first.y);
}

QPair<Vector2D, Vector2D> Canvas::getBox() {
    if (vertices.isEmpty()) {
        return {Vector2D(0, 0), Vector2D(200, 200)};
    }
    Vector2D infLeft = vertices.first();
    Vector2D supRight = vertices.first();
    for (const auto &v : vertices) {
        if (v.x < infLeft.x) infLeft.x = v.x;
        if (v.y < infLeft.y) infLeft.y = v.y;
        if (v.x > supRight.x) supRight.x = v.x;
        if (v.y > supRight.y) supRight.y = v.y;
    }
    return {infLeft, supRight};
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    // Ensure proper usage of QMouseEvent methods
    if (!event) return; // Safety check for null pointers

    // Search for a drone that is landed
    auto it = mapDrones->begin();
    while (it != mapDrones->end() && (*it)->getStatus() != Drone::landed) {
        ++it;
    }

    // If found, ask for a motion to the mouse position
    if (it != mapDrones->end()) {
        (*it)->setGoalPosition(Vector2D(event->pos().x(), event->pos().y()));
        (*it)->start();
    }
    repaint();
}
