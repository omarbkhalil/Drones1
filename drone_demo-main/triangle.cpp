#include "triangle.h"
#include <QThread>

void Triangle::computeCircle() {
    Vector2D AB = *ptr[1]-*ptr[0];
    Vector2D AC = *ptr[2]-*ptr[0];
    //OBprim= OA+0.5 AC
    Vector2D OBprim = *ptr[0] + 0.5*AC;
    Vector2D VAC = AC.orthoNormed();

    double k = (AB*AB - AC*AB)/(2*(VAC*AB));

    circumCenter = OBprim + k * VAC;
    circumRadius = (circumCenter-ptr[0]).length();
}

void Triangle::draw(QPainter &painter) {
    qDebug() << "Entered Triangle::draw";

    QPen pen(Qt::black);
    pen.setWidth(3);
    painter.setPen(pen);

    QColor c = isDelaunay ? Qt::cyan : (flippable ? Qt::gray : Qt::yellow);
    if (isHighlighted()) {
        float h, s, l;
        c.getHslF(&h, &s, &l);
        c.setHslF(h, s, l * 0.75);
        qDebug() << "Triangle is highlighted. Adjusted color brightness.";
    }
    painter.setBrush(c);

    QPointF points[3];
    for (int i = 0; i < 3; i++) {
        if (!ptr[i]) {
            qDebug() << "Error: Null pointer encountered for vertex" << i;
            return;  // Abort drawing
        }

        points[i].setX(ptr[i]->x);
        points[i].setY(ptr[i]->y);
        qDebug() << "Vertex" << i << ": (" << ptr[i]->x << "," << ptr[i]->y << ")";
    }

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




void Triangle::flippIt(QVector<Triangle>& triangles) {
    // Debugging: log the initial state
    qDebug() << "Attempting to flip a triangle.";
    QVector<const Vector2D*> commonEdges;

    bool flipCompleted = false; // Flag to control when the flip is done
    int i = 0;

    // Iterate over all neighboring triangles using while loop
    while (i < triangles.size() && !flipCompleted) {
        Triangle& tri = triangles[i];

        // Check if triangle meets flippable criteria
        if (tri.isFlippable() && &tri != this && tri.contains(this->getOpposite())) {
            // More debugging: log the state of triangles being considered
            qDebug() << "Checking triangle for flipping:" << tri.ptr[0] << tri.ptr[1] << tri.ptr[2];
            // Check each of the 3 edges to find a common edge
            int edgeIndex = 0;

            // Use nested while loop to check edges
            while (edgeIndex < 3 && !flipCompleted) {
                if (edgeIndex == 0 && tri.hasEdge(*this->getVertexPtr(1), *this->getVertexPtr(0))) {
                    // Common edge is (1, 0)
                    commonEdges = {this->getVertexPtr(1), this->getVertexPtr(0)};
                } else if (edgeIndex == 1 && tri.hasEdge(*this->getVertexPtr(2), *this->getVertexPtr(1))) {
                    // Common edge is (2, 1)
                    commonEdges = {this->getVertexPtr(2), this->getVertexPtr(1)};
                } else if (edgeIndex == 2 && tri.hasEdge(*this->getVertexPtr(0), *this->getVertexPtr(2))) {
                    // Common edge is (0, 2)
                    commonEdges = {this->getVertexPtr(0), this->getVertexPtr(2)};
                }

                // If a common edge is found, perform the flip
                if (!commonEdges.isEmpty()) {
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

                    this->computeCircle();
                    tri.computeCircle();
                    flipCompleted = true; // Set flag to indicate flip is done
                    // Debugging: log successful flip
                    qDebug() << "Flip completed between triangles.";
                }

                edgeIndex++;
            }
        }

        i++;
    }

    // If no flip occurred, still recompute the circle
    if (!flipCompleted) {
        this->computeCircle();
        // Debugging: log no flip condition
        qDebug() << "No flip was possible.";
    }

    qDebug() << "FlippIt ended.";
}



/*
void Triangle::flippIt(QVector<Triangle*> triangles) {
    QVector<const Vector2D*> commonEdges;

    qDebug() << "OPPOSITE : " << *(this->getOpposite());
    // Iterate over all neighboring triangles
    for (auto tri : triangles) {
        if (  tri->isFlippable() && tri != this  && tri->contains(this->getOpposite())) {
            // Check each of the 3 edges to find a common edge with the adjacent triangle
            if (tri->hasEdge(this->getVertexPtr(1), this->getVertexPtr(0))) {
                // Common edge is (1, 0) in the current triangle, and the third vertex is (2)

                commonEdges = {getVertexPtr(1), getVertexPtr(0)};

                // Flip both triangles by updating their vertices
                this->updateVertices(
                    this->getOpposite(),
                    const_cast<Vector2D*>(commonEdges[0]),
                    tri->getOpposite()
                    );

                tri->updateVertices(
                    tri->getOpposite(),
                    const_cast<Vector2D*>(commonEdges[1]),
                    this->getOpposite()
                    );

                tri->computeCircle();

                // Exit loop after flipping both triangles
                return;  // No need to keep searching, the flip is complete
            } else if (tri->hasEdge(getVertexPtr(2), getVertexPtr(1))) {
                // Common edge is (2, 1) in the current triangle, and the third vertex is (0)

                commonEdges = {getVertexPtr(2), getVertexPtr(1)};

                // Flip both triangles by updating their vertices
                this->updateVertices(
                    this->getOpposite(),
                    const_cast<Vector2D*>(commonEdges[0]),
                    tri->getOpposite()
                    );

                tri->updateVertices(
                    tri->getOpposite(),
                    const_cast<Vector2D*>(commonEdges[1]),
                    this->getOpposite()
                    );


                tri->computeCircle();

                // Exit loop after flipping both triangles
                return;  // No need to keep searching, the flip is complete
            } else if (tri->hasEdge(getVertexPtr(0), getVertexPtr(2))) {
                // Common edge is (0, 2) in the current triangle, and the third vertex is (1)

                commonEdges = {getVertexPtr(0), getVertexPtr(2)};

                // Flip both triangles by updating their vertices
                this->updateVertices(
                    this->getOpposite(),
                    const_cast<Vector2D*>(commonEdges[0]),
                    tri->getOpposite()
                    );

                tri->updateVertices(
                    tri->getOpposite(),
                    const_cast<Vector2D*>(commonEdges[1]),
                    this->getOpposite()
                    );

                tri->computeCircle();

                // Exit loop after flipping both triangles
                break;  // No need to keep searching, the flip is complete
            }
        }
    }
    this->computeCircle();


    qDebug() << "FlippIt ended";
}
*/
