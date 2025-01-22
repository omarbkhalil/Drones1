#ifndef VORONOI_H
#define VORONOI_H

#include <QVector>
#include <QLineF>
#include <QPainter>
#include "vector2d.h"
#include "triangle.h"

class Voronoi {
private:
    QVector<QLineF> edges;  ///< Stores the edges of the Voronoi cell
    Vector2D center;        ///< The center point of the Voronoi cell (e.g., London server)

public:
    Voronoi(const Vector2D& centerPoint);

    /**
     * @brief Generates the Voronoi edges for the specified center point based on triangles
     * @param triangles List of all Delaunay triangles
     */
    void generate(const QVector<Triangle>& triangles);

    /**
     * @brief Renders the Voronoi cell on the given QPainter
     * @param painter Reference to the QPainter for rendering
     */
    void draw(QPainter& painter) const;

    /**
     * @brief Gets the generated edges for the Voronoi cell
     * @return A QVector of QLineF representing the edges
     */
    QVector<QLineF> getEdges() const;
};

#endif // VORONOI_H
