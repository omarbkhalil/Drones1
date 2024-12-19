#ifndef SERVER_H
#define SERVER_H

#include <QString>
#include <QVector>
#include "vector2d.h"

class Server {
public:
    Server(const QString &name, const Vector2D &position, const QString &color)
        : name(name), position(position), color(color) {}

    // Getters
    QString getName() const { return name; }
    Vector2D getPosition() const { return position; }
    QString getColor() const { return color; }


    // Setters
    void setPosition(const Vector2D &pos) { position = pos; }
    void setColor(const QString &col) { color = col; }



private:
    QString name;                 ///< Name of the server
    Vector2D position;            ///< Position of the server
    QString color;                ///< Color of the server

};

#endif // SERVER_H

