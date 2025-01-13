#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <QPainter>
#include <vector2d.h>
#include "determinant.h"
#include <QDebug>
#include <QVector>

class Triangle {
private:
    Vector2D *ptr[3]; ///< array of 3 pointers on the vertices
    Vector2D circumCenter; ///< the center of the triangle (by computeCenter)
    float circumRadius; ///< the radius of the circumCircle (by computeCenter)
    QBrush brush; ///< current brush to draw the triangle
    bool isHighlited=false; ///< is drawn highlighted
    bool isDelaunay=false; ///< is drawn as Delaunay triangle
    bool flippable=false; ///< is drawn as flippable triangle
    Vector2D *flippPoint = nullptr; ///< Point making this triangle flippable
    /**
     * @brief Compute the circum circle from the vertices
     */
    void computeCircle();


public:
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

    inline bool isFlippable() {
        return flippable;
    }

    inline Vector2D getCircleCenter() {
        return circumCenter;
    }
    inline void setDelaunay(bool v, bool f) {
        isDelaunay=v;
        flippable=f;
    }

    inline void setOpposite(Vector2D *o){
        flippPoint = o;
    }

    inline Vector2D* getOpposite(){
        return flippPoint;
    }
    /**
     * @brief  set properly the isDelaunay
     * variable of each triangle according to the definition
     * and returns true if all triangles are Delaunay compliant.
     *
     */

    bool checkDelaunay(const QVector<Vector2D> &tabVertices) {
        auto it = tabVertices.begin();
        bool isOk = true;


        // Assume tabVertices has at least three points, A, B, and C
        // Assume tabVertices has at least three points, A, B, and C
        const Vector2D *A = ptr[0];
        const Vector2D *B = ptr[1];
        const Vector2D *C = ptr[2];

        while (it != tabVertices.end() && isOk) {
            Matrix33 mat;
            // PAGE 35 DU COURS GEOMETRIC ALGOITHMS
            const Vector2D D = (*it);
            mat.m[0][0] = A->x - D.x;
            mat.m[0][1] = A->y - D.y;
            mat.m[0][2] = ((A->x * A->x) - (D.x * D.x)) + ((A->y * A->y) - (D.y * D.y));

            mat.m[1][0] = B->x - D.x;
            mat.m[1][1] = B->y - D.y ;
            mat.m[1][2] = ((B->x * B->x) - (D.x * D.x)) + ((B->y * B->y) - (D.y * D.y));

            mat.m[2][0] =   C->x - D.x;
            mat.m[2][1] =  C->y - D.y ;
            mat.m[2][2] = ((C->x * C->x) - (D.x * D.x)) + ((C->y * C->y) - (D.y * D.y));

            isOk = (mat.determinant() <= 0);
            it++;
        };
        isDelaunay=isOk;
        flippable=false;
        //qDebug() << isDelaunay;
        return isDelaunay;


    }

    Vector2D* getVertexPtr(int i) const{
        return ptr[i];
    }

    bool hasEdge(  Vector2D A, Vector2D B) const{
        return (A==ptr[0] && B==ptr[1]) || (A==ptr[1] && B==ptr[2]) || (A==ptr[2] && B==ptr[0]); //|| (A==ptr[1] && B==ptr[0]) || (A==ptr[2] && B==ptr[1]) || (A==ptr[0] && B==ptr[2]);

    }


    bool contains(const Vector2D &M){
        return M==(*ptr[0]) ||   M==(*ptr[1]) ||  M==(*ptr[2]);
    }

    void updateVertices( Vector2D *_A,  Vector2D *_B,  Vector2D *_C){
        ptr[0] = _A;
        ptr[1] = _B;
        ptr[2] =_C;
    }

    bool circleContains(const Vector2D *M){
        Matrix33 mat;
        Vector2D *A = ptr[0];
        Vector2D *B = ptr[1];
        Vector2D *C = ptr[2];

        mat.m[0][0] = A->x - M->x;
        mat.m[0][1] = A->y - M->y;
        mat.m[0][2] = ((A->x * A->x) - (M->x * M->x)) + ((A->y * A->y) - (M->y * M->y));

        mat.m[1][0] = B->x - M->x;
        mat.m[1][1] = B->y - M->y ;
        mat.m[1][2] = ((B->x * B->x) - (M->x * M->x)) + ((B->y * B->y) - (M->y * M->y));

        mat.m[2][0] =   C->x - M->x;
        mat.m[2][1] =  C->y - M->y ;
        mat.m[2][2] = ((C->x * C->x) - (M->x * M->x)) + ((C->y * C->y) - (M->y * M->y));

        return mat.determinant()<=0;
    }
    /**
     * @brief Flip the triangle with an adjacent triangle if possible.
     *
     * This function attempts to flip the current triangle with one of its neighboring triangles.
     * A flip is only possible if the two triangles share a common edge, and the flip follows Delaunay's criteria.
     *
     * The function checks each of the three edges of the current triangle to find a common edge with an adjacent triangle.
     * If the flip is successful, the vertices of both triangles are updated, and the circumcircle for both triangles is recalculated.
     *
     * @param triangles The list of neighboring triangles to check for a possible flip.
     */
    void flippIt(QVector<Triangle>& triangles);
};

#endif // TRIANGLE_H
