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
    : QWidget(parent),
    myPolygon(100)
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

void Canvas::generateTriangles() {
    // Clear old data
    triangles.clear();
    polygons.clear();

    if (vertices.size() < 3) {
        qDebug() << "Not enough vertices to form a polygon.";
        return;
    }

    // Create a new polygon using the vertices
    MyPolygon* polygon = new MyPolygon(vertices.size());
    for (const auto& vertex : vertices) {
        polygon->addVertex(vertex.x, vertex.y);
    }

    // Ensure the polygon is in counter-clockwise order
    //polygon->ensureCCW();

    // Retrieve vertices and the count from the polygon
    int numVertices;
    Vector2D* polygonVertices = polygon->getVertices(numVertices);

    // Create triangles using the first vertex and every subsequent pair of vertices
    if (numVertices >= 3) {
        Vector2D* firstVertex = &polygonVertices[0];
        for (int i = 1; i < numVertices - 1; ++i) {
            // Explicitly specify the color to use the first constructor
            triangles.append(Triangle(firstVertex, &polygonVertices[i], &polygonVertices[i + 1], Qt::yellow));
        }
    }

    // Store the polygon for rendering
    polygons.push_back(polygon);

    // Debug: Log triangle vertices
    for (const Triangle& tri : triangles) {
        qDebug() << "Triangle vertices: ("
                 << tri.getVertexPtr(0)->x << "," << tri.getVertexPtr(0)->y << "), ("
                 << tri.getVertexPtr(1)->x << "," << tri.getVertexPtr(1)->y << "), ("
                 << tri.getVertexPtr(2)->x << "," << tri.getVertexPtr(2)->y << ")";
    }

    qDebug() << "Triangulation generated.";

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
        // Skip self
        if (&otherTri == &tri) continue;

        // Find common edges
        int commonCount = 0;
        Vector2D *commonA = nullptr;
        Vector2D *commonB = nullptr;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (*tri.getVertexPtr(i) == *otherTri.getVertexPtr(j)) {
                    if (commonCount == 0)
                        commonA = tri.getVertexPtr(i);
                    else if (commonCount == 1)
                        commonB = tri.getVertexPtr(i);

                    commonCount++;
                }
            }
        }

        // If exactly 2 vertices are shared, it's a common edge
        if (commonCount == 2) {
            for (int i = 0; i < 3; i++) {
                const Vector2D *v = otherTri.getVertexPtr(i);
                if (v != commonA && v != commonB) {
                    list.append(v);
                    break;
                }
            }
        }
    }

    return list;
}

void Canvas::flippAll()
{
    bool delaunay = false;

    while (!delaunay) {
        delaunay = true;

        for (Triangle &tri : triangles) {
            if (!tri.checkDelaunay(vertices)) {
                tri.flippIt(triangles);
                delaunay = false;
            }
        }
    }

    qDebug() << "All triangles are now Delaunay.";
    update();
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

/*void Canvas::generateEarClippingTriangles()
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

    // Extract triangles from the polygon and store them in Canvas::triangles
    for (const Triangle &tri : polygon->getTriangles()) {
        triangles.append(tri); // Add each triangle to Canvas' triangle list
    }

    // Store the polygon for rendering
    polygons.push_back(polygon);

    // Debug: Log triangle vertices
    for (const Triangle &tri : triangles) {
        qDebug() << "Triangle vertices: ("
                 << tri.getVertexPtr(0)->x << "," << tri.getVertexPtr(0)->y << "), ("
                 << tri.getVertexPtr(1)->x << "," << tri.getVertexPtr(1)->y << "), ("
                 << tri.getVertexPtr(2)->x << "," << tri.getVertexPtr(2)->y << ")";
    }

    qDebug() << "Ear-clipping triangulation generated.";

    update();
}

*/
void Canvas::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
   // 1) fill background
    painter.fillRect(rect(), Qt::white);

    // 2) apply transformations
    painter.translate(10, 10);
    painter.scale(scaleFactor, scaleFactor);
    painter.translate(-origin.x, -origin.y);

    myPolygon.draw(painter, true);  // Assuming draw handles its own brush and pen settings


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
    if (showCircles) {
        for (auto &tri : triangles) {
            if (tri.isHighlighted()) tri.drawCircle(painter);
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

    float canvasX = (event->pos().x() - 10) / scaleFactor + origin.x;
    float canvasY = (event->pos().y() - 10) / scaleFactor + origin.y;
    Vector2D clickPosition(canvasX, canvasY);

    qDebug() << "Mouse clicked at screen coordinates:" << event->pos().x() << event->pos().y();
    qDebug() << "Transformed to canvas coordinates:" << canvasX << canvasY;

    // Check if the click is inside any triangle
    for (Triangle &tri : triangles) {
        qDebug() << "Checking triangle with vertices: ("
                 << tri.getVertexPtr(0)->x << "," << tri.getVertexPtr(0)->y << "), ("
                 << tri.getVertexPtr(1)->x << "," << tri.getVertexPtr(1)->y << "), ("
                 << tri.getVertexPtr(2)->x << "," << tri.getVertexPtr(2)->y << ")";
        if (tri.isInside(clickPosition)) {
            qDebug() << "Point is inside the triangle!";
            tri.setHighlighted(tri.isInside(canvasX, canvasY));
            //    tri.flippIt(triangles); // Attempt to flip the clicked triangle
            update(); // Repaint after the change
            return;
        } else {
            qDebug() << "Point is outside the triangle.";
        }
    }

    qDebug() << "No triangle clicked.";

    // Original functionality: Handle drones if no triangle was clicked
    auto it = mapDrones->begin();
    while (it != mapDrones->end() && (*it)->getStatus() != Drone::landed) {
        ++it;
    }

    if (it != mapDrones->end()) {
        // Set the clicked position as the goal for the landed drone
        (*it)->setGoalPosition(Vector2D(event->pos().x(), event->pos().y()));
        (*it)->start();
    }

    update();
}

/*void Canvas::mouseMoveEvent(QMouseEvent *event) {
    float mouseX = static_cast<float>(event->pos().x() - 10) / scale + origin.x;
    float mouseY = -static_cast<float>(event->pos().y() - height() + 10) / scale + origin.y;
    emit updateSB(QString("Mouse position= (") + QString::number(mouseX, 'f', 1) + "," + QString::number(mouseY, 'f', 1) + ")");

    for (auto &tri : triangles) {
        tri.setHighlighted(tri.isInside(mouseX, mouseY));
    }
    update();
}
*/

bool Canvas::handleTriangleClick(const Vector2D &clickPosition)
{
    for (Triangle &tri : triangles) {
        if (tri.isInside(clickPosition)) {
            qDebug() << "Triangle clicked!";
            //     tri.flippIt(triangles); // Attempt to flip the clicked triangle
            return true; // Triangle was clicked
        }
    }
    qDebug() << "No triangle clicked.";
    return false; // No triangle was clicked
}

void Canvas::handleDroneClick(const QPoint &screenPos)
{
    // Find a landed drone
    auto it = mapDrones->begin();
    while (it != mapDrones->end() && (*it)->getStatus() != Drone::landed) {
        ++it;
    }

    if (it != mapDrones->end()) {
        // Set the clicked position as the goal for the landed drone
        (*it)->setGoalPosition(Vector2D(screenPos.x(), screenPos.y()));
        (*it)->start();
    }
}


Server* Canvas::findServerByName(const QString &name) {
    for (Server* srv : servers) {
        if (srv->getName() == name) {
            return srv;
        }
    }
    return nullptr;
}
void Canvas::setPolygon(const MyPolygon& polygon) {
    myPolygon = polygon;
    update();  // Optionally, trigger a repaint whenever a new polygon is set
}
