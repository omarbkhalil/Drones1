#ifndef SERVER_H
#define SERVER_H

#include <QString>
#include <QVector>
#include "drone.h"
#include "qpoint.h"
#include "vector2d.h"

class Server {
public:
    Server(const QString &name, const Vector2D &position, const QString &color)
        : name(name), position(position), color(color) {}
    int vertexId;
    QString color;
    // Getters
    QString getName() const { return name; }
    Vector2D getPosition() const { return position; }
    QString getColor() const { return color; }


    // Setters
    void setPosition(const Vector2D &pos) { position = pos; }
    void setColor(const QString &col) { color = col; }

    Server(const QString &name, QPointF location) : name(name), location(location) {}

    void addNeighbor(Server* neighbor) {
        if (!neighbors.contains(neighbor)) {
            neighbors.append(neighbor);
        }
    }

    const QList<Server*>& getNeighbors() const {
        return neighbors;
    }


    QPointF getLocation() const {
        return location;
    }
    void addDrone(Drone* drone) {
        drones.append(drone);  // Correct use of QVector's append method
    }

    QVector<Drone*> getDrones() const {
        return drones;
    }
private:
    QString name;                 ///< Name of the server
    Vector2D position;            ///< Position of the server
    QList<Server*> neighbors;
    QPointF location;
    QVector<Drone*> drones; // Using QVector to store pointers to Drone objects.

};

#endif // SERVER_H

