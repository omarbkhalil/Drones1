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
#include "voronoi.h"

/**
 * @class Canvas
 * @brief The Canvas class provides a widget that is capable of drawing and managing geometric shapes, drones, and Voronoi diagrams.
 *
 * This class integrates functionalities for drawing polygons, triangles, Voronoi diagrams, and drone images.
 * It supports interactive features such as mouse events to manipulate the shapes and elements on the canvas.
 */

class Canvas : public QWidget {
    Q_OBJECT
public:
    bool showCenters; ///< If true, centers of triangles will be shown.
    bool showDelaunay; ///< If true, Delaunay triangulation will be shown.
    bool showCircles; ///< If true, circumcircles will be shown.

    bool showTriangles = true;///< If true, triangles will be shown by default.


    const int droneIconSize = 64; ///< size of the drone picture in the canvas
    const double droneCollisionDistance = droneIconSize * 1.5; ///< distance to detect collision with other drone

    //QVector<Vector2D> triangleVertices; // Add this to store triangle points
    QVector<MyPolygon*> polygons;  /// to store ear-clipped polygons

    /**
     * @brief Constructor for Canvas.
     * @param parent The parent widget of this canvas.
     */
    explicit Canvas(QWidget *parent = nullptr);

    /**
     * @brief Destructor for Canvas.
     */

    ~Canvas();

    /**
     * @brief Initializes the Voronoi structure with a center point.
     * @param center The center point for the Voronoi initialization.
     */
    void initializeVoronoi(const Vector2D& center);  // New method for initializing Voronoi


    void setMap(QMap<QString, Drone *> *map) { mapDrones = map; } ///< Sets the map of drones.
    // void setServerPositions(const QVector<Vector2D> &positions) { serverPositions = positions; }
    void setServers(const QVector<Server *> &serverList) { servers = serverList; }///< Sets the list of server objects.

    inline int getSizeofV() { return vertices.size();}///< Returns the number of vertices.
    inline int getSizeofT() { return triangles.size();}///< Returns the number of triangles.

    //void addTriangle(int id0, int id1, int id2, const QColor &color) ;
    QVector<const Vector2D*> findOppositePointOfTrianglesWithEdgeCommon(Triangle tri);
    QVector<const Vector2D *> findOppositePointOfTriangle(Triangle &tri);
    const Vector2D* findOppositeVertex(const Triangle& tri, const Triangle& other, const QVector<const Vector2D*>& commonVertices) ;
    void flipEdge(Triangle& tri1, Triangle& tri2) ;
    bool isFlippableEdge(const Triangle& tri1, const Triangle& tri2, QVector<const Vector2D*>& commonVertices) ;     void findOppositePointOfSharedEdgeD(const Triangle &tri, const Triangle &otherTri) ;
    QVector<const Vector2D*> findCommonEdgeVertices(const Triangle& tri, const Triangle& otherTri) ;
    void addTriangle(int id0, int id1, int id2) ;
    void addTriangle(const Vector2D &v1, const Vector2D &v2, const Vector2D &v3, const QColor &color) ;

    bool checkDelaunay();///< Checks if the current triangulation is Delaunay.
    void clearTriangles(); ///< Clears all triangles from the canvas.
    void flippAll();///< Flips all flippable edges.

    void setPolygon(const MyPolygon& polygon);///< Sets the current polygon to be drawn.


    void generateSimpleTriangles() ;///< Generates simple triangles.

    void clear();///< Clears all graphical elements from the canvas.
    void addPoints(const QVector<Vector2D> &tab); ///< Adds points to the canvas.
    void generateEarClippingTriangles();///< Generates triangles using ear clipping.

    //void loadMesh(const QString &filePath);
    Server* findServerByName(const QString& name) ;///< Finds a server by its name.

    void drawTrianglesWithOppositeVerticesCheck() ; ///< Draws triangles with checks for opposite vertices.
    QVector<Vector2D> computeCircumcenters(); ///< Compute circumcenters of all triangles
    QMap<QPair<const Vector2D*, const Vector2D*>, QVector<const Triangle*>> mapEdgesToTriangles(); ///< Map edges to neighboring triangles
    QVector<QLineF> generateVoronoiEdges(); ///< Generate Voronoi edges
    QMap<Server*, QVector<QLineF>> generateVoronoiCells(); ///< Generate Voronoi
    QVector<Server*> getServers() const;  ///< Getter for servers
    void generateVoronoi();       ///< Generate Voronoi for all servers
    QVector<QLineF> getVoronoiEdges() const;  ///< Getter for Voronoi edges
signals:
    void updateSB(QString s); ///< Signal to update the status bar.

protected:
    void paintEvent(QPaintEvent *) override;///< Handles the painting of the canvas.
    void mouseMoveEvent(QMouseEvent *event) override; ///< Handles mouse move events.
    void mousePressEvent(QMouseEvent *event) override; ///< Handles mouse press events.
    void resizeEvent(QResizeEvent *event) override; ///< Handles resize events.

private:
    QPair<Vector2D, Vector2D> getBox(); ///< Calculates the bounding box.
    void reScale();///< Rescales the canvas elements.
    void generateTriangles();///< Generates the triangles.
    float scaleFactor = 1.0f; ///< The scale factor for drawing.
    MyPolygon myPolygon; ///< Polygon to be drawn.

    QVector<Triangle> triangles;///< List of triangles.
    QVector<Vector2D> vertices;///< List of vertices.
    QVector<Server *> servers;///< List of servers.
    QVector<Vector2D> serverPositions;///< Positions of servers.
    QMap<QString, Drone *> *mapDrones = nullptr;///< Map of drones.
    QImage droneImg;  ///< Image of the drone.
    float scale = 1.0f;///< Scaling factor for the canvas.
    Vector2D origin;///< Origin point for transformations.
   Voronoi* voronoi;///< Pointer to Voronoi structure.
     QVector<QLineF> voronoiEdges;///< List of Voronoi edges.
    bool handleTriangleClick(const Vector2D &clickPosition); ///< Handles triangle flipping on click
    void handleDroneClick(const QPoint &screenPos);///< Handles drone clicks.
};

#endif // CANVAS_H
