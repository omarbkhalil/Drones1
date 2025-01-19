#include "triangle.h"
#include <QThread>
#include <QPen>
#include <QPainter>
#include <QDebug>

//-------------------------------------
void Triangle::computeCircle()
{

    // We assume ptr[0], ptr[1], ptr[2] are valid
    Vector2D AB = *ptr[1] - *ptr[0];
    Vector2D AC = *ptr[2] - *ptr[0];

    // OBprim = OA + 0.5 * AC
    Vector2D OBprim = *ptr[0] + 0.5f * AC;
    Vector2D VAC    = AC.orthoNormed();

    // k = (AB*AB - AC*AB) / (2 * (VAC*AB))
    double k = (AB * AB - AC * AB) / (2.0 * (VAC * AB));

    circumCenter = OBprim + k * VAC;
    circumRadius = (circumCenter - ptr[0]).length();
}

//-------------------------------------
bool Triangle::isOnTheLeft(const Vector2D *P, const Vector2D *P1, const Vector2D *P2)
{
    Vector2D AB = *P2 - *P1;
    Vector2D AP = *P  - *P1;
    return (AB.x * AP.y - AB.y * AP.x) >= 0;
}

//-------------------------------------
bool Triangle::isInside(const Vector2D &P)
{
    // Store results of checks in variables
    bool left1 = isOnTheLeft(&P, ptr[0], ptr[1]);
    bool left2 = isOnTheLeft(&P, ptr[1], ptr[2]);
    bool left3 = isOnTheLeft(&P, ptr[2], ptr[0]);

    // Log the results for debugging
    qDebug() << "Point:" << P.x << P.y
             << "Left1:" << left1 << "Left2:" << left2 << "Left3:" << left3;

    // Return true only if all checks are true
    return left1 && left2 && left3;  // Point is inside only if all checks pass
}


//-------------------------------------
bool Triangle::hasEdge(Vector2D A, Vector2D B) const
{
    return (A == ptr[0] && B == ptr[1]) ||
           (A == ptr[1] && B == ptr[2]) ||
           (A == ptr[2] && B == ptr[0]);
}

//-------------------------------------
bool Triangle::contains(const Vector2D &M) const {
    // Check if the point is one of the vertices
    if ((M == *ptr[0]) || (M == *ptr[1]) || (M == *ptr[2])) {
        return true;
    }

    // Check if the point lies on any of the edges
    if (isOnTheEdge(M, *ptr[0], *ptr[1]) ||
        isOnTheEdge(M, *ptr[1], *ptr[2]) ||
        isOnTheEdge(M, *ptr[2], *ptr[0])) {
        return true;
    }

    return false;
}

bool Triangle::isOnTheEdge(const Vector2D &P, const Vector2D &A, const Vector2D &B) const {
    // Check if P is collinear with A and B and lies within the segment [A, B]
    Vector2D AB = B - A;
    Vector2D AP = P - A;

    // Check collinearity using cross product
    if (std::abs(AB.x * AP.y - AB.y * AP.x) > 1e-6) {
        return false;
    }

    // Check if the point is within the segment
    float dotProduct = AP.x * AB.x + AP.y * AB.y;
    float squaredLengthAB = AB.x * AB.x + AB.y * AB.y;

    return dotProduct >= 0 && dotProduct <= squaredLengthAB;
}

//-------------------------------------
bool Triangle::circleContains(const Vector2D *M)
{
    // Build a 3x3 determinant
    Matrix33 mat;
    Vector2D *A = ptr[0];
    Vector2D *B = ptr[1];
    Vector2D *C = ptr[2];

    mat.m[0][0] = A->x - M->x;
    mat.m[0][1] = A->y - M->y;
    mat.m[0][2] = (A->x*A->x - M->x*M->x) + (A->y*A->y - M->y*M->y);

    mat.m[1][0] = B->x - M->x;
    mat.m[1][1] = B->y - M->y;
    mat.m[1][2] = (B->x*B->x - M->x*M->x) + (B->y*B->y - M->y*M->y);

    mat.m[2][0] = C->x - M->x;
    mat.m[2][1] = C->y - M->y;
    mat.m[2][2] = (C->x*C->x - M->x*M->x) + (C->y*C->y - M->y*M->y);

    return (mat.determinant() <= 0);
}

//-------------------------------------
bool Triangle::checkDelaunay(const QVector<Vector2D> &tabVertices)
{
    bool isOk = true;
    Vector2D* A = ptr[0];
    Vector2D* B = ptr[1];
    Vector2D* C = ptr[2];

    for (const Vector2D &D : tabVertices) {
        Matrix33 mat;

        mat.m[0][0] = A->x - D.x;
        mat.m[0][1] = A->y - D.y;
        mat.m[0][2] = (A->x * A->x - D.x * D.x) + (A->y * A->y - D.y * D.y);

        mat.m[1][0] = B->x - D.x;
        mat.m[1][1] = B->y - D.y;
        mat.m[1][2] = (B->x * B->x - D.x * D.x) + (B->y * B->y - D.y * D.y);

        mat.m[2][0] = C->x - D.x;
        mat.m[2][1] = C->y - D.y;
        mat.m[2][2] = (C->x * C->x - D.x * D.x) + (C->y * C->y - D.y * D.y);

        // If determinant > 0, it means D is inside the circumcircle
        if (mat.determinant() > 0) {
            isOk = false;
            qDebug() << "Point inside circumcircle. Not Delaunay.";
            break;
        }
    }

    isDelaunay = isOk;
    flippable = !isOk;
    return isOk;
}


//-------------------------------------
void Triangle::updateVertices(Vector2D *_A, Vector2D *_B, Vector2D *_C)
{
    ptr[0] = _A;
    ptr[1] = _B;
    ptr[2] = _C;
    computeCircle();
}

//-------------------------------------
void Triangle::draw(QPainter &painter) {
    qDebug() << "Entered Triangle::draw";

    // Set the pen properties
    QPen pen(Qt::black);
    pen.setWidth(3);
    painter.setPen(pen);

    // Determine the fill color
    QColor color = isDelaunayTriangle() ? Qt::cyan : (flippable ? Qt::gray : Qt::yellow);

    // Adjust brightness if highlighted
    if (isHighlighted()) {
        float h, s, l;
        color.getHslF(&h, &s, &l);
        color.setHslF(h, s, l * 0.75f);
        qDebug() << "Triangle is highlighted. Adjusted color brightness.";
    }

    painter.setBrush(color);

    // Prepare points for the triangle
    QPointF points[3];
    for (int i = 0; i < 3; i++) {
        if (!ptr[i]) {
            qDebug() << "Error: Null pointer encountered for vertex" << i;
            return;  // Abort drawing if any vertex is null
        }

        points[i].setX(ptr[i]->x);
        points[i].setY(ptr[i]->y);
        qDebug() << "Vertex" << i << ": (" << ptr[i]->x << "," << ptr[i]->y << ")";
    }

    // Draw the triangle
    try {
        painter.drawPolygon(points, 3);
        qDebug() << "Successfully drew triangle.";
    } catch (...) {
        qDebug() << "Error while drawing triangle.";
    }
}

void Triangle::drawCircle(QPainter &painter) {
    painter.setPen(QPen(Qt::black,3,Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(circumCenter.x-circumRadius,circumCenter.y-circumRadius,2.0*circumRadius,2.0*circumRadius);
}

//-------------------------------------
void Triangle::flippIt(QVector<Triangle> &triangles)
{
    qDebug() << "Attempting to flip a triangle.";

    QVector<const Vector2D*> commonEdges;

    for (Triangle &tri : triangles) {
        // Skip self
        if (&tri == this) continue;

        // Clear previous common edges
        commonEdges.clear();

        // Check for a common edge between this triangle and tri
        for (int i = 0; i < 3; i++) {
            if (tri.hasEdge(*this->getVertexPtr(i), *this->getVertexPtr((i + 1) % 3))) {
                commonEdges.append(this->getVertexPtr(i));
                commonEdges.append(this->getVertexPtr((i + 1) % 3));
                break;
            }
        }

        // If a common edge is found, attempt flipping
        if (commonEdges.size() == 2) {
            qDebug() << "Found common edge between triangles: ("
                     << commonEdges[0]->x << "," << commonEdges[0]->y << ") and ("
                     << commonEdges[1]->x << "," << commonEdges[1]->y << ")";

            // Validate opposite points before proceeding
            if (!this->getOpposite() || !tri.getOpposite()) {
                qDebug() << "Error: Opposite points are not set or invalid.";
                continue;
            }

            // Ensure the opposite points are not part of the shared edge
            if (this->getOpposite() == commonEdges[0] || this->getOpposite() == commonEdges[1] ||
                tri.getOpposite() == commonEdges[0] || tri.getOpposite() == commonEdges[1]) {
                qDebug() << "Error: Opposite points are part of the shared edge. Skipping.";
                continue;
            }

            qDebug() << "Flipping edge with opposite points: ("
                     << this->getOpposite()->x << "," << this->getOpposite()->y << ") and ("
                     << tri.getOpposite()->x << "," << tri.getOpposite()->y << ")";

            // Perform the flip by updating vertices
            this->updateVertices(
                this->getOpposite(),
                const_cast<Vector2D*>(commonEdges[0]),
                tri.getOpposite()
                );

            tri.updateVertices(
                tri.getOpposite(),
                const_cast<Vector2D*>(commonEdges[1]),
                this->getOpposite()
                );

            // Recompute circumcircles for both triangles
            this->computeCircle();
            tri.computeCircle();

            qDebug() << "Flip completed.";
            return;
        }
    }

    qDebug() << "No flip performed.";
}

