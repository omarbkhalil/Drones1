#ifndef SERVER_H
#define SERVER_H

#include <QString>
#include <QVector>
#include "drone.h"
#include "qpoint.h"
#include "vector2d.h"


/**
 * @class Server
 * @brief Represents a server that can manage drones and communicate with other servers.
 *
 * The Server class encapsulates all functionalities related to a server managing drones
 * within a specific area. It stores information about the server's name, position, color,
 * and relationships with neighboring servers and drones.
 */
class Server {
public:

    /**
     * @brief Construct a new Server object with specified attributes.
     *
     * @param name Name of the server.
     * @param position Geographical position of the server.
     * @param color Color identifier for the server.
     */
    Server(const QString &name, const Vector2D &position, const QString &color)
        : name(name), position(position), color(color) {}


    int vertexId;///< Identifier for the server's position vertex in a graph.
    QString color;///< Color identifier for the server.
    // Getters
    // Getters
    /**
     * @brief Get the name of the server.
     *
     * @return QString The name of the server.
     */
    QString getName() const { return name; }
    /**
     * @brief Get the position of the server.
     *
     * @return Vector2D The position of the server.
     */
    Vector2D getPosition() const { return position; }
    /**
     * @brief Get the color of the server.
     *
     * @return QString The color of the server.
     */
    QString getColor() const { return color; }


    // Setters
    /**
     * @brief Set the position of the server.
     *
     * @param pos New position of the server.
     */
    void setPosition(const Vector2D &pos) { position = pos; }

    /**
     * @brief Set the color of the server.
     *
     * @param col New color of the server.
     */
    void setColor(const QString &col) { color = col; }


    /**
     * @brief Construct a new Server object with a name and 2D location.
     *
     * @param name Name of the server.
     * @param location Location of the server using a QPointF.
     */
    Server(const QString &name, QPointF location) : name(name), location(location) {}

    /**
     * @brief Add a neighboring server to this server.
     *
     * @param neighbor Pointer to the neighboring server.
     */
    void addNeighbor(Server* neighbor) {
        if (!neighbors.contains(neighbor)) {
            neighbors.append(neighbor);
        }
    }

    /**
     * @brief Get a list of neighboring servers.
     *
     * @return const QList<Server*>& List of neighboring servers.
     */
    const QList<Server*>& getNeighbors() const {
        return neighbors;
    }

    /**
     * @brief Get the location of the server.
     *
     * @return QPointF The 2D point representing the server's location.
     */
    QPointF getLocation() const {
        return location;
    }

    /**
     * @brief Add a drone to the server's management.
     *
     * @param drone Pointer to the drone to be added.
     */
    void addDrone(Drone* drone) {
        drones.append(drone);  // Correct use of QVector's append method
    }


    /**
     * @brief Get the list of drones managed by this server.
     *
     * @return QVector<Drone*> List of drones.
     */
    QVector<Drone*> getDrones() const {
        return drones;
    }
private:
    QString name;                 ///< Name of the server
    Vector2D position;            ///< Position of the server
    QList<Server*> neighbors;///< List of neighboring servers
    QPointF location;///< 2D location of the server
    QVector<Drone*> drones; ///< List of drones managed by this server

};

#endif // SERVER_H

