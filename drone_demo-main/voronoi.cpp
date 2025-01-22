#include "voronoi.h"
#include <QDebug>

Voronoi::Voronoi(const Vector2D& centerPoint) : center(centerPoint) {}

void Voronoi::generate(const QVector<Triangle>& triangles) {
    edges.clear();

    // Iterate through all triangles to find those containing the center point
    for (const Triangle& triangle : triangles) {
        if (triangle.contains(center)) {
            // Add edges connecting the circumcenters of neighboring triangles
            for (int i = 0; i < 3; ++i) {
                const Vector2D* v1 = triangle.getVertexPtr(i);
                const Vector2D* v2 = triangle.getVertexPtr((i + 1) % 3);

                // Find neighboring triangle sharing this edge
                for (const Triangle& neighbor : triangles) {
                    if (&triangle != &neighbor && neighbor.hasEdge(*v1, *v2)) {
                        QLineF edge(triangle.getCircleCenter().x,
                                    triangle.getCircleCenter().y,
                                    neighbor.getCircleCenter().x,
                                    neighbor.getCircleCenter().y);
                        edges.append(edge);
                        break;
                    }
                }
            }
        }
    }

    qDebug() << "Generated Voronoi edges for point:" << center.x << center.y
             << "Edges count:" << edges.size();
}

void Voronoi::draw(QPainter& painter) const {
    painter.setPen(QPen(Qt::blue, 2));
    for (const QLineF& edge : edges) {
        painter.drawLine(edge);
    }
}

QVector<QLineF> Voronoi::getEdges() const {
    return edges;
}
