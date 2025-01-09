#include "triangle.h"
#include <iostream>
#include <ostream>

void Triangle::computeCircle() {
    auto A=ptr[0];
    auto B=ptr[1];
    auto C=ptr[2];
    auto AC = (*C-*A);
    auto AB = (*B-*A);
    auto Bprim = A+0.5*AC;
    double k = (AB*AB - AC*AB)/(2* (AC.orthoNormed()*AB));

    circumCenter = Bprim + k*AC.orthoNormed();
    circumRadius = (circumCenter-ptr[0]).length();

}

void Triangle::draw(QPainter &painter) {
    std::cout << "Drawing triangle at vertices: "
              << ptr[0]->x << ", " << ptr[0]->y << " - "
              << ptr[1]->x << ", " << ptr[1]->y << " - "
              << ptr[2]->x << ", " << ptr[2]->y << std::endl;

    QPen pen(Qt::black);
    pen.setWidth(3);
    painter.setPen(pen);
    painter.setBrush(QColor(0, 255, 0)); // Bright green for high visibility
    QPointF points[3];
    for (int i = 0; i < 3; i++) {
        points[i] = QPointF(ptr[i]->x, ptr[i]->y);
    }
    painter.drawPolygon(points, 3);
}



void Triangle::drawCircle(QPainter &painter) {
    painter.setPen(QPen(Qt::black,3,Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(circumCenter.x-circumRadius,circumCenter.y-circumRadius,2.0*circumRadius,2.0*circumRadius);
}

bool Triangle::contains(Vector2D &point)
{
    return ptr[0]==point || ptr[1]==point || ptr[2]==point;
}
