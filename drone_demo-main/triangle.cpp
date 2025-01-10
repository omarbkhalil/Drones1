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
    QPen pen(Qt::black);
    pen.setWidth(3);
    painter.setPen(pen);
    painter.setBrush(isHighlited?(isDelaunay?Qt::green:Qt::red):Qt::yellow);
    QPointF points[3];
    for (int i=0; i<3; i++) {
        points[i].setX(ptr[i]->x);
        points[i].setY(ptr[i]->y);
    }
    painter.drawPolygon(points,3);
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
