#include "vector2d.h"

const Vector2D operator *(double a,const Vector2D &v) {
    return Vector2D(a*v.x,a*v.y);
}

double operator *(const Vector2D &u,const Vector2D &v) {
    return u.x*v.x+u.y*v.y;
}

const Vector2D operator +(const Vector2D &u,const Vector2D &v) {
    return Vector2D(u.x+v.x,u.y+v.y);
}

const Vector2D operator -(const Vector2D &u,const Vector2D &v) {
    return Vector2D(u.x-v.x,u.y-v.y);
}

const Vector2D operator-(const Vector2D &v) {
    return Vector2D(-v.x,-v.y);
}

double operator ^(const Vector2D &u,const Vector2D &v) {
    return u.x*v.y-u.y*v.x;
}

bool operator==(const Vector2D &u,const Vector2D &v) {
    return (u.x==v.x && u.y==v.y);
}
uint qHash(const Vector2D& key, uint seed) {
    // Combine the x and y values into a single hash value
    return qHash(QPair<double, double>(key.x, key.y), seed);
}
