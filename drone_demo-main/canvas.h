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
    bool showCircles = false;

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
    QVector<const Vector2D*> findOppositePointOfTrianglesWithEdgeCommon(Triangle tri);
    QVector<const Vector2D *> findOppositePointOfTriangle(Triangle &tri);
    const Vector2D* findOppositeVertex(const Triangle& tri, const Triangle& other, const QVector<const Vector2D*>& commonVertices) ;
    void flipEdge(Triangle& tri1, Triangle& tri2) ;
    bool isFlippableEdge(const Triangle& tri1, const Triangle& tri2, QVector<const Vector2D*>& commonVertices) ;     void findOppositePointOfSharedEdgeD(const Triangle &tri, const Triangle &otherTri) ;
    QVector<const Vector2D*> findCommonEdgeVertices(const Triangle& tri, const Triangle& otherTri) ;
    void addTriangle(int id0, int id1, int id2) ;
    void addTriangle(const Vector2D &v1, const Vector2D &v2, const Vector2D &v3, const QColor &color) ;

    bool checkDelaunay();
    void clearTriangles();
    void flippAll();

    void setPolygon(const MyPolygon& polygon);

    void generateSimpleTriangles() ;

    void clear();
    void addPoints(const QVector<Vector2D> &tab);
    void generateEarClippingTriangles();

    //void loadMesh(const QString &filePath);
    Server* findServerByName(const QString& name) ;

    void drawTrianglesWithOppositeVerticesCheck() ;
    QVector<Vector2D> computeCircumcenters(); ///< Compute circumcenters of all triangles
    QMap<QPair<const Vector2D*, const Vector2D*>, QVector<const Triangle*>> mapEdgesToTriangles(); ///< Map edges to neighboring triangles
    QVector<QLineF> generateVoronoiEdges(); ///< Generate Voronoi edges
    QMap<Server*, QVector<QLineF>> generateVoronoiCells(); ///< Generate Vor
signals:
    void updateSB(QString s);

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPair<Vector2D, Vector2D> getBox();
    void reScale();
    void generateTriangles();
    float scaleFactor = 1.0f;
    MyPolygon myPolygon;  // Polygon to be drawn

    QVector<Triangle> triangles;
    QVector<Vector2D> vertices;
    QVector<Server *> servers;
    QVector<Vector2D> serverPositions;
    QMap<QString, Drone *> *mapDrones = nullptr;
    QImage droneImg;
    float scale = 1.0f;
    Vector2D origin;

    bool handleTriangleClick(const Vector2D &clickPosition); ///< Handles triangle flipping on click
    void handleDroneClick(const QPoint &screenPos);
};

#endif // CANVAS_H
