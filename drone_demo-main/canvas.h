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
    void paintEvent(QPaintEvent*) override;
    /**
     * @brief mousePressEvent
     * @param event
     */
    void mousePressEvent(QMouseEvent *event) override;
signals:

private:
    QMap<QString,Drone*> *mapDrones=nullptr; ///< pointer on the map of the drones
    QImage droneImg; ///< picture representing the drone in the canvas
};

#endif // CANVAS_H
