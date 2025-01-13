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
    return isOnTheLeft(&P, ptr[0], ptr[1]) &&
           isOnTheLeft(&P, ptr[1], ptr[2]) &&
           isOnTheLeft(&P, ptr[2], ptr[0]);
}

//-------------------------------------
bool Triangle::hasEdge(Vector2D A, Vector2D B) const
{
    return (A == ptr[0] && B == ptr[1]) ||
           (A == ptr[1] && B == ptr[2]) ||
           (A == ptr[2] && B == ptr[0]);
}

//-------------------------------------
bool Triangle::contains(const Vector2D &M)
{
    return (M == *ptr[0]) || (M == *ptr[1]) || (M == *ptr[2]);
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
        mat.m[0][2] = (A->x*A->x - D.x*D.x) + (A->y*A->y - D.y*D.y);

        mat.m[1][0] = B->x - D.x;
        mat.m[1][1] = B->y - D.y;
        mat.m[1][2] = (B->x*B->x - D.x*D.x) + (B->y*B->y - D.y*D.y);

        mat.m[2][0] = C->x - D.x;
        mat.m[2][1] = C->y - D.y;
        mat.m[2][2] = (C->x*C->x - D.x*D.x) + (C->y*C->y - D.y*D.y);

        // If determinant > 0, it means D is inside the circumcircle
        if (mat.determinant() > 0) {
            isOk = false;
            break;
        }
    }

    // if isOk is true, no points found inside -> it's Delaunay
    // else false
    isDelaunay = isOk;
    flippable  = false;
    return isDelaunay;
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
void Triangle::draw(QPainter &painter)
{
    qDebug() << "Entered Triangle::draw";

    QPen pen(Qt::black);
    pen.setWidth(3);
    painter.setPen(pen);

    // Example logic: If isDelaunay, color it cyan; if flippable, color it gray; otherwise, yellow
    QColor c = (isDelaunayTriangle() ? Qt::cyan
                                     : (flippable ? Qt::gray : Qt::yellow));
    if (isHighlighted()) {
        float h, s, l;
        c.getHslF(&h, &s, &l);
        c.setHslF(h, s, l * 0.75f);
        qDebug() << "Triangle is highlighted. Adjusted color brightness.";
    }
    painter.setBrush(c);

    QPointF points[3];
    for (int i = 0; i < 3; i++) {
        if (!ptr[i]) {
            qDebug() << "Error: Null pointer encountered for vertex" << i;
            return;  // abort drawing
        }

        points[i].setX(ptr[i]->x);
        points[i].setY(ptr[i]->y);
        qDebug() << "Vertex" << i << ": (" << ptr[i]->x << "," << ptr[i]->y << ")";
    }

    try {
        painter.drawPolygon(points, 3);
        qDebug() << "Successfully drew triangle.";
    }
    catch (...) {
        qDebug() << "Error while drawing triangle.";
    }
}

//-------------------------------------
void Triangle::drawCircle(QPainter &painter)
{
    painter.setPen(QPen(Qt::black, 3, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(circumCenter.x - circumRadius,
                        circumCenter.y - circumRadius,
                        2.f * circumRadius,
                        2.f * circumRadius);
}

//-------------------------------------
void Triangle::flippIt(QVector<Triangle> &triangles)
{
    qDebug() << "Attempting to flip a triangle.";

    QVector<const Vector2D*> commonEdges;
    bool flipCompleted = false;
    int i = 0;

    while (i < triangles.size() && !flipCompleted) {
        Triangle &tri = triangles[i];

        // Must not be the same triangle, must contain my 'opposite' point
        if (tri.isFlippable() && &tri != this && tri.contains(*this->getOpposite())) {
            qDebug() << "Checking triangle for flipping:" << tri.ptr[0] << tri.ptr[1] << tri.ptr[2];

            int edgeIndex = 0;
            while (edgeIndex < 3 && !flipCompleted) {
                if (edgeIndex == 0 && tri.hasEdge(*this->getVertexPtr(1), *this->getVertexPtr(0))) {
                    commonEdges = {this->getVertexPtr(1), this->getVertexPtr(0)};
                }
                else if (edgeIndex == 1 && tri.hasEdge(*this->getVertexPtr(2), *this->getVertexPtr(1))) {
                    commonEdges = {this->getVertexPtr(2), this->getVertexPtr(1)};
                }
                else if (edgeIndex == 2 && tri.hasEdge(*this->getVertexPtr(0), *this->getVertexPtr(2))) {
                    commonEdges = {this->getVertexPtr(0), this->getVertexPtr(2)};
                }

                if (!commonEdges.isEmpty()) {
                    // Perform flip
                    this->updateVertices(this->getOpposite(),
                                         const_cast<Vector2D*>(commonEdges[0]),
                                         tri.getOpposite());
                    tri.updateVertices(tri.getOpposite(),
                                       const_cast<Vector2D*>(commonEdges[1]),
                                       this->getOpposite());

                    flipCompleted = true;
                    qDebug() << "Flip completed between triangles.";
                }
                edgeIndex++;
            }
        }
        i++;
    }

    // If no flip occurred, just recompute the circle to be sure
    if (!flipCompleted) {
        computeCircle();
        qDebug() << "No flip was possible.";
    }

    qDebug() << "FlippIt ended.";
}
