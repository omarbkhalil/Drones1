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
#include "mypolygon.h"

class Canvas : public QWidget {
    Q_OBJECT
public:
    bool showCenters = false;
    bool showDelaunay = false;
    bool showTriangles = true;

    const int droneIconSize = 64; ///< size of the drone picture in the canvas
    const double droneCollisionDistance = droneIconSize * 1.5; ///< distance to detect collision with other drone

    //QVector<Vector2D> triangleVertices; // Add this to store triangle points
    QVector<MyPolygon*> polygons;  // to store ear-clipped polygons


    explicit Canvas(QWidget *parent = nullptr);
    ~Canvas();

    void setMap(QMap<QString, Drone *> *map) { mapDrones = map; }
   // void setServerPositions(const QVector<Vector2D> &positions) { serverPositions = positions; }
    void setServers(const QVector<Server *> &serverList) { servers = serverList; }

    inline int getSizeofV() { return vertices.size();}
    inline int getSizeofT() { return triangles.size();}

    //void addTriangle(int id0, int id1, int id2, const QColor &color) ;
    QVector<const Vector2D*> findOppositePointOfTrianglesWithEdgeCommon(const Triangle &tri);
    QVector<const Vector2D *> findOppositePointOfTriangle(Triangle &tri);

    void addTriangle(int id0, int id1, int id2) ;
    void addTriangle(const Vector2D &v1, const Vector2D &v2, const Vector2D &v3, const QColor &color) ;

    bool checkDelaunay();
    void clearTriangles();
    void flippAll();


    void generateSimpleTriangles() ;

    void clear();
    void addPoints(const QVector<Vector2D> &tab);
     void generateEarClippingTriangles();

    //void loadMesh(const QString &filePath);
    Server* findServerByName(const QString& name) ;
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPair<Vector2D, Vector2D> getBox();
    void reScale();
    void generateTriangles();
    float scaleFactor = 1.0f;

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
