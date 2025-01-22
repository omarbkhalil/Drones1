/**
 * @file triangle.h
 * @brief This file defines the Triangle class and its associated methods.
 */

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <QPainter>
#include <vector2d.h>
#include "determinant.h"
#include <QDebug>
#include <QVector>

/**
 * @class Triangle
 * @brief The Triangle class represents a triangle with three vertices. It can store information about
 *        the triangle's circumcircle, its Delaunay status, and perform various geometry-related checks.
 */
class Triangle
{
private:
    /**
     * @brief circumCenter
     * The center of the triangle (calculated by computeCircle).
     */
    Vector2D  circumCenter;///< the center of the triangle (calculated by computeCircle)

    /**
     * @brief circumRadius
     * The radius of the circumcircle for this triangle.
     */
    float     circumRadius;///< the radius of the circumCircle

    /**
     * @brief flippable
     * Indicates whether this triangle is eligible for a flip operation during triangulation.
     */
    bool      flippable   = false;  ///< whether triangle is flippable

    /**
     * @brief isDelaunay
     * Internal flag that indicates if the triangle is Delaunay.
     * Made private for proper encapsulation. Use `isDelaunayTriangle()` to check it.
     */
    bool      isDelaunay  = false;

    /**
     * @brief flippPoint
     * Pointer to a vertex that might be used during flipping logic.
     */
    Vector2D* flippPoint  = nullptr;

    /**
     * @brief computeCircle
     * Computes the circumcircle from the three vertices and sets `circumCenter` and `circumRadius`.
     */
    void computeCircle();

public:
    /**
     * @brief ptr
     * Array of 3 pointers to the vertices of this triangle.
     */
    Vector2D* ptr[3];      ///< array of 3 pointers to the vertices

    /**
     * @brief brush
     * The current brush used to draw the triangle.
     */
    QBrush    brush;       ///< current brush to draw the triangle

    /**
     * @brief isHighlited
     * Indicates whether the triangle is highlighted for drawing purposes.
     */
    bool      isHighlited = false;  ///< whether triangle is highlighted

    /**
     * @brief triangles
     * A static list containing all triangles.
     */
    static QVector<Triangle> triangles; // Static list of all triangles

    /**
     * @brief Constructs a Triangle with three vertices and a specified color.
     * @param v0 Pointer to the first vertex.
     * @param v1 Pointer to the second vertex.
     * @param v2 Pointer to the third vertex.
     * @param color The color to be used for the triangle's brush.
     */
    Triangle(Vector2D* v0, Vector2D* v1, Vector2D* v2, const QColor &color) {
        ptr[0] = v0;
        ptr[1] = v1;
        ptr[2] = v2;
        brush.setColor(color);
        computeCircle(); // Optionally compute properties like circumcircle
    }

    /**
     * @brief Default constructor. Initializes vertex pointers to nullptr.
     */
    Triangle() {
        ptr[0] = nullptr;
        ptr[1] = nullptr;
        ptr[2] = nullptr;
        // Initialize other necessary members, if any.
    }

    /**
     * @brief setTriangles
     * Updates the static `triangles` list with the given set of Triangles.
     * @param tris A QVector of Triangle objects to set.
     */
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

    /**
     * @brief Constructs a Triangle with three vertices, using a default color (yellow).
     * @param ptr1 Pointer to the first vertex.
     * @param ptr2 Pointer to the second vertex.
     * @param ptr3 Pointer to the third vertex.
     */
    Triangle(Vector2D* ptr1, Vector2D* ptr2, Vector2D* ptr3)
    {
        ptr[0] = ptr1;
        ptr[1] = ptr2;
        ptr[2] = ptr3;
        setColor(Qt::yellow);
        computeCircle();
    }

    /**
     * @brief isDelaunayTriangle
     * @return True if this triangle is flagged as Delaunay, otherwise false.
     */
    inline bool isDelaunayTriangle() const
    {
        return isDelaunay;
    }

    /**
     * @brief setDelaunay
     * Sets the isDelaunay and flippable flags.
     * @param v The new value for the Delaunay flag.
     * @param f The new value for the flippable flag.
     */
    inline void setDelaunay(bool v, bool f)
    {
        isDelaunay = v;
        flippable  = f;
    }

    /**
     * @brief isFlippable
     * @return True if the triangle is marked as flippable.
     */
    bool isFlippable() const {
        qDebug() << "Checking flippable status for triangle: ("
                 << ptr[0]->x << "," << ptr[0]->y << "), ("
                 << ptr[1]->x << "," << ptr[1]->y << "), ("
                 << ptr[2]->x << "," << ptr[2]->y << ")";
        return flippable;
    }

    /**
     * @brief setOpposite
     * Sets the flippPoint (opposite vertex) for the triangle.
     * @param o Pointer to the opposite point vertex.
     */
    inline void setOpposite(Vector2D* o)
    {
        if (o) {
            qDebug() << "Setting opposite point:" << o->x << o->y;
        } else {
            qDebug() << "Setting opposite point: nullptr";
        }
        flippPoint = o;
    }

    /**
     * @brief getOpposite
     * Retrieves the opposite vertex given a set of common edges.
     * @param commonEdges A QVector of pointers to the vertices forming the common edges.
     * @return A pointer to the vertex opposite to the common edges, or nullptr if none found.
     */
    Vector2D* getOpposite(const QVector<const Vector2D*>& commonEdges) const;

    /**
     * @brief setHighlighted
     * Sets the highlight status of the triangle.
     * @param v True if the triangle should be highlighted, otherwise false.
     */
    inline void setHighlighted(bool v)
    {
        isHighlited = v;
    }

    /**
     * @brief isHighlighted
     * @return True if the triangle is highlighted, otherwise false.
     */
    inline bool isHighlighted() const
    {
        return isHighlited;
    }

    /**
     * @brief getVertexPtr
     * Retrieves a pointer to the i-th vertex of the triangle.
     * @param i Index of the vertex (0, 1, or 2).
     * @return Pointer to the vertex at index i.
     */
    inline Vector2D* getVertexPtr(int i) const
    {
        return ptr[i];
    }

    /**
     * @brief getCircleCenter
     * @return The circumcenter of the triangle.
     */
    inline Vector2D getCircleCenter() const
    {
        return circumCenter;
    }

    /**
     * @brief setColor
     * Sets the color (and style) of the triangle's brush.
     * @param p_color The new color to be set.
     */
    void setColor(const QColor &p_color)
    {
        brush.setStyle(Qt::SolidPattern);
        brush.setColor(p_color);
    }

    /**
     * @brief isOnTheLeft
     * Checks if a point P is on the left side of the segment from P1 to P2.
     * @param P Pointer to the point being tested.
     * @param P1 Pointer to the first segment endpoint.
     * @param P2 Pointer to the second segment endpoint.
     * @return True if P is on the left of the segment P1->P2, otherwise false.
     */
    bool isOnTheLeft(const Vector2D *P, const Vector2D *P1, const Vector2D *P2);

    /**
     * @brief isInside
     * Determines whether the point P lies inside the triangle.
     * @param P The point to test.
     * @return True if the point is inside this triangle, otherwise false.
     */
    bool isInside(const Vector2D &P);

    /**
     * @brief isInside
     * Determines whether the point with coordinates (x,y) lies inside the triangle.
     * @param x The x-coordinate of the point.
     * @param y The y-coordinate of the point.
     * @return True if the point is inside this triangle, otherwise false.
     */
    inline bool isInside(float x, float y) { return isInside(Vector2D(x, y)); }

    /**
     * @brief hasEdge
     * Checks if this triangle has an edge defined by the points A and B.
     * @param A The first point of the edge.
     * @param B The second point of the edge.
     * @return True if the triangle has the edge AB, otherwise false.
     */
    bool hasEdge(Vector2D A, Vector2D B) const;

    /**
     * @brief contains
     * Checks if the triangle contains the point M.
     * @param M The point to check.
     * @return True if M is inside this triangle, otherwise false.
     */
    bool contains(const Vector2D &M) const;

    /**
     * @brief isOnTheEdge
     * Checks if point P lies on the edge from A to B.
     * @param P The point to check.
     * @param A The starting point of the edge.
     * @param B The ending point of the edge.
     * @return True if P is on the edge AB, otherwise false.
     */
    bool isOnTheEdge(const Vector2D &P, const Vector2D &A, const Vector2D &B) const ;

    /**
     * @brief flippIt
     * Performs a flip operation on this triangle, updating the provided set of triangles as needed.
     * @param triangles A QVector of Triangles that may be affected by the flip operation.
     */
    void flippIt(QVector<Triangle>& triangles);

    /**
     * @brief circleContains
     * Checks if a point M is inside or on this triangle's circumcircle.
     * @param M Pointer to the point being tested.
     * @return True if M is inside or on the circumcircle, otherwise false.
     */
    bool circleContains(const Vector2D* M);

    /**
     * @brief checkDelaunay
     * Updates `isDelaunay` based on whether any point in `tabVertices` lies inside the circumcircle.
     * @param tabVertices A QVector of points to test against the circumcircle.
     * @return True if no points lie inside the circumcircle (triangle remains Delaunay), otherwise false.
     */
    bool checkDelaunay(const QVector<Vector2D> &tabVertices);

    /**
     * @brief updateVertices
     * Reassigns the 3 vertex pointers (usually used by flipping operations).
     * @param _A Pointer to the new first vertex.
     * @param _B Pointer to the new second vertex.
     * @param _C Pointer to the new third vertex.
     */
    void updateVertices( Vector2D *_A,  Vector2D *_B,  Vector2D *_C);

    /**
     * @brief draw
     * Draws the triangle on the provided QPainter.
     * @param painter A reference to the QPainter on which to draw this triangle.
     */
    void draw(QPainter &painter) const;

    /**
     * @brief drawCircle
     * Draws the triangle's circumcircle on the provided QPainter.
     * @param painter A reference to the QPainter on which to draw the circumcircle.
     */
    void drawCircle(QPainter &painter);

    /**
     * @brief flippIt
     * Overloaded function (empty body) possibly a placeholder for another flipping operation.
     */
    void flippIt();
};

#endif // TRIANGLE_H
