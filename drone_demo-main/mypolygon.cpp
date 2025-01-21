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

Vector2D *MyPolygon::getVertices(int &n)
{
    n = N;
    return tabPts;
}

void MyPolygon::draw(QPainter &painter, bool showTriangles) const {
    if (N < 3) {
        // Not enough points to form a polygon
        return;
    }

    // Draw the polygon outline
    QPolygonF qPolygon;
    for (int i = 0; i < N; i++) {
        qPolygon << QPointF(tabPts[i].x, tabPts[i].y);
    }

    painter.setBrush(currentColor);
    painter.setPen(QPen(Qt::black, 1));  // Setting pen width for better visibility
    painter.drawPolygon(qPolygon);

    // Draw triangles if requested
    if (showTriangles && !triangles.empty()) {
        for (const Triangle& tri : triangles) {
            QPolygonF triPolygon;
            triPolygon << QPointF(tri.ptr[0]->x, tri.ptr[0]->y)
                       << QPointF(tri.ptr[1]->x, tri.ptr[1]->y)
                       << QPointF(tri.ptr[2]->x, tri.ptr[2]->y)
                       << QPointF(tri.ptr[0]->x, tri.ptr[0]->y); // Closing the triangle loop

            // Set brush and pen based on triangle properties
            painter.setBrush(tri.brush);
            QPen pen = (tri.isHighlited) ? QPen(Qt::red, 2, Qt::SolidLine) : QPen(Qt::black, 1, Qt::SolidLine);
            painter.setPen(pen);
            painter.drawPolygon(triPolygon);
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

    // Copy vertices to a modifiable container
    QVector<Vector2D*> poly;
    for (int i = 0; i < N; i++) {
        poly.append(&tabPts[i]);
    }

    if (poly.size() < 3) {
        qDebug() << "Not enough points to form a polygon.";
        return;
    }

    // Ear-clipping process
    while (poly.size() > 3) {
        bool earFound = false;

        // Attempt to find an ear among all vertices
        for (int i = 0; i < poly.size(); i++) {
            if (isEar(poly, i)) {
                int iPrev = (i - 1 + poly.size()) % poly.size();
                int iNext = (i + 1) % poly.size();

                // Construct a triangle using pointers to existing vertices
                Triangle tri(poly[iPrev], poly[i], poly[iNext], Qt::yellow);

                // Set opposite points for flippable edges
                if (!triangles.isEmpty()) {
                    triangles.last().setOpposite(poly[iNext]);
                    tri.setOpposite(poly[iPrev]);
                }

                triangles.push_back(tri);

                // Remove the ear tip from the polygon
                poly.removeAt(i);
                earFound = true;
                break;
            }
        }

        if (!earFound) {
            qDebug() << "Ear not found. Polygon may be self-intersecting or invalid.";
            return; // Exit if the polygon is invalid
        }
    }

    // Final triangle
    if (poly.size() == 3) {
        Triangle tri(poly[0], poly[1], poly[2], Qt::yellow);
        triangles.push_back(tri);
    }

    qDebug() << "Ear clipping done. Triangles formed:" << triangles.size();
}




// --------------------------------------------------
// Checks if poly[i] is an ear
// --------------------------------------------------
bool MyPolygon::isEar(const QVector<Vector2D*> &poly, int i) const
{
    int n = poly.size();
    int iPrev = (i - 1 + n) % n;
    int iNext = (i + 1) % n;

    const Vector2D *A = poly[iPrev];
    const Vector2D *B = poly[i];
    const Vector2D *C = poly[iNext];

    Vector2D AB = *B - *A;
    Vector2D BC = *C - *B;
    float crossVal = AB.x * BC.y - AB.y * BC.x;
    if (crossVal < 0.0f) {
        return false; // Reflex angle, not an ear
    }

    for (int j = 0; j < n; j++) {
        if (j == iPrev || j == i || j == iNext) continue;
        if (pointInTriangle(*poly[j], *A, *B, *C)) {
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

    return (std::fabs(areaABC - (areaPAB + areaPBC + areaPCA)) < 1e-5);
}
void MyPolygon::computeConvexHull() {
    QVector<Vector2D> points(tabPts, tabPts + N);  // Assuming points are already loaded into tabPts
    QVector<Vector2D> hull;

    // Sort points
    std::sort(points.begin(), points.end(), [](const Vector2D& a, const Vector2D& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });

    // Build lower hull
    for (int i = 0; i < points.size(); ++i) {
        while (hull.size() >= 2 && ((hull.back() - hull[hull.size() - 2]) ^ (points[i] - hull.back())) <= 0)
            hull.pop_back();
        hull.push_back(points[i]);
    }

    // Build upper hull
    int t = hull.size() + 1;
    for (int i = points.size() - 1; i >= 0; --i) {
        while (hull.size() >= t && ((hull.back() - hull[hull.size() - 2]) ^ (points[i] - hull.back())) <= 0)
            hull.pop_back();
        hull.push_back(points[i]);
    }

    hull.pop_back();  // Remove the duplicate of the first point
    // Update the MyPolygon with hull points
    N = hull.size();
    for (int i = 0; i < N; ++i) {
        tabPts[i] = hull[i];
    }
}

void MyPolygon::addInteriorPoint(const Vector2D& point) {
    interiorPoints.push_back(point);
}

void MyPolygon::integrateInteriorPoints() {
    QVector<Triangle> newTriangles;

    // Iterate through each original triangle
    for (const Triangle& originalTri : triangles) {
        QVector<Vector2D*> remainingPoints;
        for (const Vector2D& pt : interiorPoints) {
            remainingPoints.push_back(new Vector2D(pt));  // Copy all interior points initially
        }

        QVector<Triangle> tempTriangles;
        tempTriangles.push_back(originalTri);  // Start with the original triangle

        // Check each triangle formed so far against each interior point
        bool splitOccurred;
        do {
            splitOccurred = false;
            QVector<Triangle> currentPassTriangles;

            for (Triangle& tri : tempTriangles) {
                bool pointUsed = false;

                for (int i = 0; i < remainingPoints.size(); ++i) {
                    if (pointInTriangle(*remainingPoints[i], *tri.ptr[0], *tri.ptr[1], *tri.ptr[2])) {
                        // Split the triangle into three new triangles
                        Triangle t1(tri.ptr[0], tri.ptr[1], remainingPoints[i], Qt::yellow);
                        Triangle t2(tri.ptr[1], tri.ptr[2], remainingPoints[i], Qt::yellow);
                        Triangle t3(tri.ptr[2], tri.ptr[0], remainingPoints[i], Qt::yellow);

                        currentPassTriangles.push_back(t1);
                        currentPassTriangles.push_back(t2);
                        currentPassTriangles.push_back(t3);

                        remainingPoints.removeAt(i);  // Remove the point that was used to split
                        pointUsed = true;
                        splitOccurred = true;
                        break;  // Break after modifying the vector
                    }
                }

                if (!pointUsed) {
                    currentPassTriangles.push_back(tri);  // Keep the triangle if no point found
                }
            }

            tempTriangles = currentPassTriangles;  // Prepare for the next pass
        } while (splitOccurred);

        // Append all triangles that have been processed
        newTriangles.append(tempTriangles);
    }

    // Replace old triangles with the new set
    triangles = newTriangles;
    qDebug() << "Updated triangulation with interior points. Total triangles: " << triangles.size();

    // Pass the updated triangles to the Triangle class
    Triangle::setTriangles(triangles);
}

