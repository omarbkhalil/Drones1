#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <QPainter>
#include <vector2d.h>
#include "determinant.h"
#include <QDebug>
#include <QVector>

class Triangle
{
private:
    // ----------------------------
    // Data Members
    // ----------------------------
    Vector2D  circumCenter;///< the center of the triangle (calculated by computeCircle)
    float     circumRadius;///< the radius of the circumCircle
    bool      flippable   = false;  ///< whether triangle is flippable

    /**
     * @brief isDelaunay - internal flag that indicates if the triangle is Delaunay.
     * Made private for proper encapsulation. Use `isDelaunayTriangle()` to check it.
     */
    bool      isDelaunay  = false;

    /**
     * @brief flippPoint - pointer to a vertex that might be used during flipping logic
     */
    Vector2D* flippPoint  = nullptr;

    /**
     * @brief computeCircle - compute the circum circle from the three vertices.
     * Sets `circumCenter` and `circumRadius`.
     */
    void computeCircle();

public:
    Vector2D* ptr[3];      ///< array of 3 pointers to the vertices
    QBrush    brush;       ///< current brush to draw the triangle
    bool      isHighlited = false;  ///< whether triangle is highlighted

    // ----------------------------
    // Constructors
    // ----------------------------
    static QVector<Triangle> triangles; // Static list of all triangles



    Triangle(Vector2D* v0, Vector2D* v1, Vector2D* v2, const QColor &color) {
        ptr[0] = v0;
        ptr[1] = v1;
        ptr[2] = v2;
        brush.setColor(color);
        computeCircle(); // Optionally compute properties like circumcircle
    }

    Triangle() {
        ptr[0] = nullptr;
        ptr[1] = nullptr;
        ptr[2] = nullptr;
        // Initialize other necessary members, if any.
    }

    static void setTriangles(const QVector<Triangle>& tris) {
        Triangle::triangles.clear();  // Clear the existing triangles
        Triangle::triangles.reserve(tris.size());  // Optimize memory allocation

        qDebug() << "Setting triangles. Total count:" << tris.size();
        for (const Triangle& incomingTriangle : tris) {
            // Create a new Triangle using the existing vertices
            Triangle newTriangle(
                incomingTriangle.ptr[0],  // Vertex 0
                incomingTriangle.ptr[1],  // Vertex 1
                incomingTriangle.ptr[2],  // Vertex 2
                incomingTriangle.brush.color()  // Retain the brush color
                );

            // Debug each vertex
            for (int i = 0; i < 3; i++) {
                if (incomingTriangle.ptr[i]) {
                    qDebug() << "Vertex" << i << ": (" << incomingTriangle.ptr[i]->x << "," << incomingTriangle.ptr[i]->y << ")";
                } else {
                    qDebug() << "Vertex" << i << ": null";
                }
            }

            // Add the new triangle to the static vector
            Triangle::triangles.append(newTriangle);
        }
    }


    Triangle(Vector2D* ptr1, Vector2D* ptr2, Vector2D* ptr3)
    {
        ptr[0] = ptr1;
        ptr[1] = ptr2;
        ptr[2] = ptr3;
        setColor(Qt::yellow);
        computeCircle();
    }

    // ----------------------------
    // Getters / Setters
    // ----------------------------

    /**
     * @brief isDelaunayTriangle
     * @return true if this triangle is flagged as Delaunay, else false
     */
    inline bool isDelaunayTriangle() const
    {
        return isDelaunay;
    }

    /**
     * @brief setDelaunay - sets the isDelaunay and flippable flags
     * @param v - new isDelaunay value
     * @param f - new flippable value
     */
    inline void setDelaunay(bool v, bool f)
    {
        isDelaunay = v;
        flippable  = f;
    }

    inline bool isFlippable() const
    {
        return flippable;
    }


    inline void setOpposite(Vector2D* o)
    {
        if (o) {
            qDebug() << "Setting opposite point:" << o->x << o->y;
        } else {
            qDebug() << "Setting opposite point: nullptr";
        }
        flippPoint = o;
    }

    inline Vector2D* getOpposite()
    {
        return flippPoint;
    }
    inline void setHighlighted(bool v)
    {
        isHighlited = v;
    }

    inline bool isHighlighted() const
    {
        return isHighlited;
    }

    inline Vector2D* getVertexPtr(int i) const
    {
        return ptr[i];
    }

    inline Vector2D getCircleCenter() const
    {
        return circumCenter;
    }

    // ----------------------------
    // Triangle geometry
    // ----------------------------
    void setColor(const QColor &p_color)
    {
        brush.setStyle(Qt::SolidPattern);
        brush.setColor(p_color);
    }

    /**
     * @brief isOnTheLeft
     * @return true if point P is on the left side of the segment from P1 to P2
     */
    bool isOnTheLeft(const Vector2D *P, const Vector2D *P1, const Vector2D *P2);

    bool isInside(const Vector2D &P);
    inline bool isInside(float x, float y) { return isInside(Vector2D(x, y)); }

    bool hasEdge(Vector2D A, Vector2D B) const;
    bool contains(const Vector2D &M) const;
    bool isOnTheEdge(const Vector2D &P, const Vector2D &A, const Vector2D &B) const ;

    /**
     * @brief circleContains
     * @return true if M is inside or on this triangle's circumcircle
     */
    bool circleContains(const Vector2D* M);

    /**
     * @brief checkDelaunay
     * Updates `isDelaunay` based on whether any point in `tabVertices` lies inside
     * the circumcircle. Returns true if it remains Delaunay, false otherwise.
     */
    bool checkDelaunay(const QVector<Vector2D> &tabVertices);

    /**
     * @brief updateVertices
     * Reassign the 3 vertex pointers. Typically used by flipping operations.
     */
    void updateVertices(Vector2D *_A, Vector2D *_B, Vector2D *_C);

    // ----------------------------
    // Drawing
    // ----------------------------
    void draw(QPainter &painter) const;
    void drawCircle(QPainter &painter);

    // ----------------------------
    // Flipping
    // ----------------------------
    void flippIt();
};

#endif // TRIANGLE_H
