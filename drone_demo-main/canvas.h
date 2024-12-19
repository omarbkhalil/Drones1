/**
 * @brief Drone_demo project
 * @author B.Piranda
 * @date dec. 2024
 **/
#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <drone.h>
#include <QMouseEvent>
#include <QPaintEvent>
#include "server.h"

class Canvas : public QWidget {
    Q_OBJECT
public:
    const int droneIconSize=64; ///< size of the drone picture in the vanvas
    const double droneCollisionDistance=droneIconSize*1.5; ///< distance to detect collision with other drone
    /**
     * @brief Canvas constructor
     * @param parent
     */
    explicit Canvas(QWidget *parent = nullptr);
    /**
     * @brief setMap set the list of drones (identified by their name) to the canvas
     * @param map the map of couple "name of the drone"/"drone pointer"
     */
    inline void setMap(QMap<QString,Drone*> *map) { mapDrones=map; }
    /**
     * @brief paintEvent
     */
    inline void setServerPositions(const QVector<Vector2D> &positions) { serverPositions = positions; }

    /**
     * @brief paintEvent override to render the canvas
     */
    void paintEvent(QPaintEvent*) override;
    /**
     * @brief mousePressEvent
     * @param event
     */
    void mousePressEvent(QMouseEvent *event) override;
    void setServers(const QVector<Server *> &serverList) { servers = serverList; }

signals:

private:
    QMap<QString,Drone*> *mapDrones=nullptr; ///< pointer on the map of the drones
    QImage droneImg; ///< picture representing the drone in the canvas
     QVector<Vector2D> serverPositions;
    QVector<Server *> servers; ///< List of servers
};

#endif // CANVAS_H
