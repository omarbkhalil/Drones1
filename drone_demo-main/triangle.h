#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "determinant.h"
#include "qdebug.h"

#include <QPainter>
#include <vector2d.h>

class Triangle {
    Vector2D *ptr[3]; ///< array of 3 pointers on the vertices
    Vector2D circumCenter; ///< the center of the triangle (by computeCenter)
    float circumRadius; ///< the radius of the circumCircle (by computeCenter)
    QBrush brush; ///< current brush to draw the triangle
    bool isHighlited=false; ///< is drawn highlighted
    bool isDelaunay=true; ///< is drawn as Delaunay triangle
    bool isFlipable=false;
    /**
     * @brief Compute the circum circle from the vertices
     */
    void computeCircle();
public:
    /**
     * @brief Triangle class constructor
     * @param ptr1 for first point
     * @param ptr2 for second point
     * @param ptr3 for third point
     * @warning points \p ptr1, \p ptr2,\p tr3 must be enumerated CCW ordered
     */
    Triangle(Vector2D *ptr1,Vector2D *ptr2,Vector2D *ptr3) {
        ptr[0]=ptr1;
        ptr[1]=ptr2;
        ptr[2]=ptr3;
        setColor(Qt::yellow);
        computeCircle();
    }

    Triangle(Vector2D *ptr1,Vector2D *ptr2,Vector2D *ptr3,const QColor &p_color) {
        ptr[0]=ptr1;
        ptr[1]=ptr2;
        ptr[2]=ptr3;
        setColor(p_color);
        computeCircle();
    }

    bool isOnTheLeft(const Vector2D *P,const Vector2D *P1,const Vector2D *P2) {
        Vector2D AB = *P2-*P1,
            AP = *P-*P1;

        return (AB.x*AP.y - AB.y*AP.x)>=0;
    }

    inline bool isInside(float x,float y) {
        return isInside(Vector2D(x,y));
    }

    bool isInside(const Vector2D &P) {
        return isOnTheLeft(&P,ptr[0],ptr[1]) &&
               isOnTheLeft(&P,ptr[1],ptr[2]) &&
               isOnTheLeft(&P,ptr[2],ptr[0]);
    }

    void draw(QPainter &painter);
    void drawCircle(QPainter &painter);

    void setColor(const QColor &p_color) {
        brush.setStyle(Qt::BrushStyle::SolidPattern);
        brush.setColor(p_color);
    }

    inline void setHighlighted(bool v) {
        isHighlited=v;
    }
    inline bool isHighlighted() {
        return isHighlited;
    }
    inline bool isFlipabled() {
        return isFlipable;
    }
    inline Vector2D getCircleCenter() {
        return circumCenter;
    }
    inline void setDelaunay(bool v,bool f) {
        isDelaunay=v;
        isFlipable=f;
    }
    bool checkDelaunay(const QVector<Vector2D>&tabVertices) {
        auto it=tabVertices.begin();
        bool isOk = true;
        Vector2D A = *ptr[0];
        Vector2D B = *ptr[1];
        Vector2D C = *ptr[2];
        while (it!=tabVertices.end() && isOk){
            Matrix33 mat;
            mat.m[0][0] = A.x-it->x;
            mat.m[0][1] = A.y-it->y;
            mat.m[0][2] = A.x*A.x - it->x*it->x + A.y*A.y - it->y*it->y;
            mat.m[1][0] = B.x-it->x;
            mat.m[1][1] = B.y-it->y;
            mat.m[1][2] = B.x*B.x - it->x*it->x + B.y*B.y - it->y*it->y;
            mat.m[2][0] = C.x-it->x;
            mat.m[2][1] = C.y-it->y;
            mat.m[2][2] = C.x*C.x - it->x*it->x + C.y*C.y - it->y*it->y;
            isOk = mat.determinant()<=0; // positive value for outside point and equal to 0 for A,B,C
            it++;
        }
        isDelaunay = isOk;
        //search points M inside the circle of the current triangle
        //check if there is a common edge triangle containing M
        return isDelaunay;
    }
    bool contains(Vector2D &point);
    const Vector2D *getVertexPtr(int i) const {
        return ptr[i];
    }
    bool hasEdge(const Vector2D* A,const Vector2D* B){
        return (A==ptr[0] && B==ptr[1]) || (A==ptr[1] && B==ptr[2]) || (A==ptr[2] && B==ptr[0]);
    }
    bool circleContains(const Vector2D * M){
        Vector2D A = *ptr[0];
        Vector2D B = *ptr[1];
        Vector2D C = *ptr[2];
        Matrix33 mat;
        mat.m[0][0] = A.x-M->x;
        mat.m[0][1] = A.y-M->y;
        mat.m[0][2] = A.x*A.x - M->x*M->x + A.y*A.y - M->y*M->y;
        mat.m[1][0] = B.x-M->x;
        mat.m[1][1] = B.y-M->y;
        mat.m[1][2] = B.x*B.x - M->x*M->x + B.y*B.y - M->y*M->y;
        mat.m[2][0] = C.x-M->x;
        mat.m[2][1] = C.y-M->y;
        mat.m[2][2] = C.x*C.x - M->x*M->x + C.y*C.y - M->y*M->y;
        return mat.determinant()<=0;
    }
};

#endif // TRIANGLE_H
