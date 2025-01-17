#ifndef MYPOLYGON_H
#define MYPOLYGON_H

#include "vector2d.h"
#include <QPainter>
#include <QDebug>
#include <QVector>
#include "triangle.h"

/**
 * @brief The MyPolygon class
 * Stores up to Nmax vertices in a CCW orientation
 * and can triangulate itself via ear clipping.
 */
class MyPolygon {
private:
    int Nmax;            ///< maximum capacity
    int N;               ///< current number of vertices
    Vector2D *tabPts;    ///< array of polygon vertices
    QColor currentColor; ///< color used to fill the polygon
    QVector<Triangle> triangles; ///< result of ear clipping

public:
    /**
     * @brief MyPolygon constructor
     * @param p_Nmax maximum number of vertices
     */
    MyPolygon(int p_Nmax);

    /**
     * @brief destructor
     */
    ~MyPolygon();

    /**
     * @brief addVertex inserts a new vertex (x,y) if we have room
     */
    void addVertex(float x, float y);

    /**
     * @brief getTriangles
     * @return a reference to the internal triangle list
     */
    const QVector<Triangle>& getTriangles() const {
        return triangles;
    }

    /**
     * @brief getBoundingBox finds the min/max corners of the polygon
     * @return QPair of bottom-left and top-right corners
     */
    QPair<Vector2D, Vector2D> getBoundingBox() const;

    /**
     * @brief setColor sets the color of the polygon fill
     */
    void setColor(const QColor c);

    /**
     * @brief changeColor placeholder to demonstrate how you might
     *        change color if a user clicks near a vertex, etc.
     */
    void changeColor(const Vector2D &pt);

    /**
     * @brief getVertices returns read-only access to the vertex array
     * @param n output param for how many vertices
     * @return pointer to the internal array of Vector2D
     */
    Vector2D *getVertices(int &n);

    /**
     * @brief draw renders the polygon and, if showTriangles==true,
     *        also renders the triangles from ear clipping
     */
    void draw(QPainter &painter, bool showTriangles) const;

    /**
     * @brief ensureCCW checks the signed area. If negative, reverses
     *        tabPts so that the polygon is in CCW order.
     */
    void ensureCCW();

    /**
     * @brief earClippingTriangulate performs ear clipping to fill
     *        'triangles' with a triangulation of this polygon.
     *        Make sure to call ensureCCW() first if needed.
     */
    void earClippingTriangulate();
void computeConvexHull();
    void addInternalPoint(const Vector2D &internalPoint);

private:
    /**
     * @brief computeSignedArea uses the shoelace formula
     * @return positive => CCW, negative => CW, zero => degenerate
     */
    double computeSignedArea() const;

    /**
     * @brief isEar checks if poly[i] is an “ear” vertex
     */
    bool isEar(const QVector<Vector2D*> &poly, int i) const;

    /**
     * @brief pointInTriangle checks if point p is inside triangle ABC
     */
    bool pointInTriangle(const Vector2D &p,
                         const Vector2D &A,
                         const Vector2D &B,
                         const Vector2D &C) const;
};

#endif // MYPOLYGON_H
