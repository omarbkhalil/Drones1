#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QVector>
#include <QMap>
#include <QImage>
#include "vector2d.h"
#include "triangle.h"
#include "drone.h"
#include "server.h"

class Canvas : public QWidget {
    Q_OBJECT
public:
    const int droneIconSize = 64; ///< size of the drone picture in the canvas
    const double droneCollisionDistance = droneIconSize * 1.5; ///< distance to detect collision with other drone

    explicit Canvas(QWidget *parent = nullptr);
    ~Canvas();

    void setMap(QMap<QString, Drone *> *map) { mapDrones = map; }
    void setServerPositions(const QVector<Vector2D> &positions) { serverPositions = positions; }
    void setServers(const QVector<Server *> &serverList) { servers = serverList; }
    void addTriangle(int id0, int id1, int id2, const QColor &color = Qt::gray);
    bool checkDelaunay();
    void clearTriangles();
    void clear();
    void addPoints(const QVector<Vector2D> &tab);
    void loadMesh(const QString &filePath);
    QVector<const Vector2D *> findOppositePointOfTriangle(Triangle &tri);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPair<Vector2D, Vector2D> getBox();
    void reScale();
    void generateTriangles();

    QVector<Triangle> triangles;
    QVector<Vector2D> vertices;
    QVector<Server *> servers;
    QVector<Vector2D> serverPositions;
    QMap<QString, Drone *> *mapDrones = nullptr;
    QImage droneImg;
    float scale = 1.0f;
    Vector2D origin;
};

#endif // CANVAS_H
