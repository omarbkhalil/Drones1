#include "mypolygon.h"
#include <algorithm> // for std::reverse
#include <cmath>     // for fabs
#include "vector2d.h"
#include <QPainter>
#include <QDebug>
#include <QVector>
#include <triangle.h>
MyPolygon::MyPolygon(int p_Nmax) : Nmax(p_Nmax)
{
    N = 0;
    tabPts = new Vector2D[Nmax];
    currentColor = Qt::green;
}

MyPolygon::~MyPolygon()
{
    delete[] tabPts;
}

void MyPolygon::addVertex(float x, float y)
{
    if (N < Nmax) {
        tabPts[N] = Vector2D(x, y);
        N++;
    } else {
        qDebug() << "Error: Max number of vertices (" << Nmax << ") reached!";
    }
}

QPair<Vector2D, Vector2D> MyPolygon::getBoundingBox() const
{
    if (N == 0) {
        return qMakePair(Vector2D(0, 0), Vector2D(0, 0));
    }

    Vector2D bottomLeft = tabPts[0];
    Vector2D topRight   = tabPts[0];

    for (int i = 1; i < N; i++) {
        if (tabPts[i].x < bottomLeft.x)  bottomLeft.x = tabPts[i].x;
        if (tabPts[i].y < bottomLeft.y)  bottomLeft.y = tabPts[i].y;
        if (tabPts[i].x > topRight.x)    topRight.x   = tabPts[i].x;
        if (tabPts[i].y > topRight.y)    topRight.y   = tabPts[i].y;
    }

    return qMakePair(bottomLeft, topRight);
}

void MyPolygon::setColor(const QColor c)
{
    currentColor = c;
}

void MyPolygon::changeColor(const Vector2D &pt)
{
    // Placeholder: you might detect if pt is inside polygon, then do something
    // For now, just log
    qDebug() << "changeColor called with pt:" << pt.x << pt.y << "(not implemented)";
}

const Vector2D *MyPolygon::getVertices(int &n) const
{
    n = N;
    return tabPts;
}

void MyPolygon::draw(QPainter &painter, bool showTriangles) const
{
    if (N < 3) {
        // Not a polygon
        return;
    }

    // 1) Draw the polygon outline
    QPolygonF qPolygon;
    for (int i = 0; i < N; i++) {
        qPolygon << QPointF(tabPts[i].x, tabPts[i].y);
    }

    painter.setBrush(currentColor);
    painter.setPen(Qt::black);

    try {
        painter.drawPolygon(qPolygon);
    } catch (...) {
        qDebug() << "Error while rendering polygon in MyPolygon::draw.";
    }

    // 2) If requested, draw the triangles that we've computed via ear clipping
    if (showTriangles) {
        painter.setPen(Qt::DashLine);
        for (const auto &triangle : triangles) {
            QPolygonF triPoly;
            for (int j = 0; j < 3; j++) {
                triPoly << QPointF(
                    triangle.getVertexPtr(j)->x,
                    triangle.getVertexPtr(j)->y
                    );
            }
            painter.drawPolygon(triPoly);
        }
    }
}

// --------------------------------------------------
// Ensure the polygon is in CCW order
// --------------------------------------------------
void MyPolygon::ensureCCW()
{
    double area = computeSignedArea();
    if (area < 0.0) {
        // It's clockwise, reverse it
        for (int i = 0; i < N/2; i++) {
            Vector2D tmp = tabPts[i];
            tabPts[i] = tabPts[N-1 - i];
            tabPts[N-1 - i] = tmp;
        }
        qDebug() << "Polygon reversed to ensure CCW orientation.";
    } else {
        qDebug() << "Polygon is already CCW (or degenerate). area =" << area;
    }
}

// --------------------------------------------------
// Compute signed area (shoelace formula).
// positive => CCW, negative => CW
// --------------------------------------------------
double MyPolygon::computeSignedArea() const
{
    if (N < 3) return 0.0;

    double area = 0.0;
    for (int i = 0; i < N; i++) {
        int j = (i + 1) % N;
        area += (tabPts[i].x * tabPts[j].y - tabPts[j].x * tabPts[i].y);
    }
    return 0.5 * area;
}

// --------------------------------------------------
// Ear clipping
// --------------------------------------------------
void MyPolygon::earClippingTriangulate()
{
    // Clear old data
    triangles.clear();

    // Copy the polygon points to a modifiable container
    QVector<Vector2D> poly;
    poly.reserve(N);
    for (int i = 0; i < N; i++) {
        poly.push_back(tabPts[i]);
    }

    if (poly.size() < 3) {
        qDebug() << "Not enough points to form a polygon.";
        return;
    }

    // We'll keep removing ears until we get down to 3 vertices
    while (poly.size() > 3) {
        bool earFound = false;

        // Attempt to find an ear among all vertices
        for (int i = 0; i < poly.size(); i++) {
            if (isEar(poly, i)) {
                int iPrev = (i - 1 + poly.size()) % poly.size();
                int iNext = (i + 1) % poly.size();

                // Construct a triangle from these 3 points
                // We'll allocate new Vector2D for each corner
                // (be mindful of memory in a bigger app)
                Vector2D* vA = new Vector2D(poly[iPrev]);
                Vector2D* vB = new Vector2D(poly[i]);
                Vector2D* vC = new Vector2D(poly[iNext]);

                Triangle tri(vA, vB, vC, Qt::red);
                triangles.push_back(tri);

                // Remove the ear tip from the polygon
                poly.remove(i);
                earFound = true;
                break;
            }
        }

        if (!earFound) {
            // Possibly the polygon is not simple or something else is wrong
            qDebug() << "Ear not found. Polygon may be self-intersecting or invalid.";
            break;
        }
    }

    // If exactly 3 left, that's our final triangle
    if (poly.size() == 3) {
        Vector2D* vA = new Vector2D(poly[0]);
        Vector2D* vB = new Vector2D(poly[1]);
        Vector2D* vC = new Vector2D(poly[2]);
        Triangle tri(vA, vB, vC, Qt::red);
        triangles.push_back(tri);
    }

    qDebug() << "Ear clipping done. Triangles formed:" << triangles.size();
}

// --------------------------------------------------
// Checks if poly[i] is an ear
// --------------------------------------------------
bool MyPolygon::isEar(QVector<Vector2D> &poly, int i) const
{
    int n = poly.size();
    int iPrev = (i - 1 + n) % n;
    int iNext = (i + 1) % n;

    const Vector2D &A = poly[iPrev];
    const Vector2D &B = poly[i];
    const Vector2D &C = poly[iNext];

    // 1) Check if ABC is "convex" for a CCW polygon => cross(AB, BC) >= 0
    Vector2D AB = B - A;
    Vector2D BC = C - B;
    float crossVal = AB.x * BC.y - AB.y * BC.x;
    if (crossVal < 0.0f) {
        // If < 0 => it's a reflex angle => not an ear
        return false;
    }

    // 2) Check that no other vertex is inside triangle (A,B,C)
    for (int j = 0; j < n; j++) {
        if (j == iPrev || j == i || j == iNext) continue;
        if (pointInTriangle(poly[j], A, B, C)) {
            return false;
        }
    }
    return true;
}

// --------------------------------------------------
// Check if point p is inside triangle ABC
// (using area-based approach)
// --------------------------------------------------
bool MyPolygon::pointInTriangle(const Vector2D &p,
                                const Vector2D &A,
                                const Vector2D &B,
                                const Vector2D &C) const
{
    auto cross = [](const Vector2D &u, const Vector2D &v) {
        return u.x * v.y - u.y * v.x;
    };

    float areaABC = std::fabs(cross(B - A, C - A));
    float areaPAB = std::fabs(cross(p - A, B - A));
    float areaPBC = std::fabs(cross(p - B, C - B));
    float areaPCA = std::fabs(cross(p - C, A - C));

    float sum = areaPAB + areaPBC + areaPCA;
    return (std::fabs(sum - areaABC) < 1e-5);
}
