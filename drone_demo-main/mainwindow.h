/**
 * @brief Drone_demo project
 * @author B.Piranda
 * @date dec. 2024
 **/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QTimer>
#include <QElapsedTimer>
#include <drone.h>
#include <vector2d.h>
#include <server.h>
#include <mypolygon.h>
#include "voronoi.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


/**
 * @class MainWindow
 * @brief The MainWindow class manages the main window of the Drone_demo application.
 *
 * This class is responsible for the graphical user interface, handling user interactions,
 * and managing the lifecycle and operations of drones within the application.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Distributes drones equally across the defined area.
     */
    void distributeDronesEqually();


    /**
     * @brief Constructs a MainWindow object.
     * @param parent The parent widget, defaulting to nullptr.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor for MainWindow.
     */
    ~MainWindow();

private slots:
    /**
     * @brief Slot to handle the "Quit" action triggered from the GUI.
     */
    void on_actionQuit_triggered();

    /**
     * @brief Regularly updates GUI elements and drone states.
     */
    void update();

    /**
     * @brief Slot to handle the "Load" action to load configuration or data files.
     */
    void on_actionLoad_triggered();

    /**
     * @brief Toggles the visibility of centers in the visualization.
     * @param checked Whether the centers should be shown or not.
     */
    void on_actionshowCenters_triggered(bool checked);
    /**
     * @brief Toggles the visibility of the Delaunay triangulation.
     * @param checked Whether the Delaunay triangulation should be shown or not.
     */
    void on_actionshowDelaunay_triggered(bool checked);
    /**
     * @brief Toggles the visibility of circumcircles in the visualization.
     * @param checked Whether the circles should be shown or not.
     */
    void on_actionshowCircles_triggered(bool checked);
    /**
     * @brief Triggers the visualization of Voronoi diagrams.
     */
    void on_actionshowVoronoi_triggered();

private:
    Ui::MainWindow *ui;///< Pointer to the user interface.
    QMap<QString,Drone*> mapDrones;///< Map of drone identifiers to Drone objects.
    QTimer *timer; ///< Timer for periodic updates and operations
    QElapsedTimer elapsedTimer;///< Timer for measuring elapsed time.
    MyPolygon *polygon;///< Pointer to a polygon used in the application.

    /**
     * @brief Loads a JSON file and parses it to initialize the application state.
     * @param filePath Path to the JSON file to be loaded.
     */
    QVector<Server *> servers; ///< List of server objects managing drone operations.
    QVector<Vector2D> allPoints; ///< List of all points used in visualizations.
    Voronoi* voronoi; ///< Pointer to the Voronoi diagram manager.

};
#endif // MAINWINDOW_H
