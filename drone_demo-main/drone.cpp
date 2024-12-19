#include "drone.h"
#include <QPainter>
#include <QStyle>
#include <QDebug>

Drone::Drone(const QString &n,QWidget *parent)
    : QWidget{parent},name(n) {

    status=landed;
    speed=0;
    power=maxPower/2.0;
    V.set(0,0);
    ForceCollision.set(0,0);
    position=Vector2D(50,50);
    goalPosition=Vector2D(550,600);
    showCollision=false;
    azimut=0;

    speedPB=new QProgressBar(this);
    speedPB->setValue(speed);
    speedPB->setMaximum(maxSpeed);
    speedPB->setMinimum(0);
    speedPB->setFormat(name+" speed %p%");
    speedPB->setAlignment(Qt::AlignCenter);
    //speedPB->setStyleSheet("QProgressBar::chunk{background-color:red");

    powerPB=new QProgressBar(this);
    powerPB->setValue(power);
    powerPB->setMaximum(maxPower);
    powerPB->setMinimum(0);
    powerPB->setFormat("power %p%");
    powerPB->setAlignment(Qt::AlignCenter);
    //powerPB->setStyleSheet("QProgressBar::chunk{background-color:yellow");

    setBaseSize(barSpace+compasSize,2*compasSize);
    setMinimumHeight(2*compasSize);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    compasImg.load("../../media/compas.png");
    stopImg.load("../../media/stop.png");
    takeoffImg.load("../../media/takeoff.png");
    landingImg.load("../../media/landing.png");
}

Drone::~Drone() {
    delete speedPB;
    delete powerPB;
}

void Drone::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QBrush whiteBrush(Qt::SolidPattern);
    whiteBrush.setColor(Qt::white);
    QRect rect(0,0,compasSize,compasSize);
    switch (status) {
        case landed: painter.drawImage(rect,stopImg); break;
        case takeoff: painter.drawImage(rect,takeoffImg); break;
        case landing: painter.drawImage(rect,landingImg); break;
        default : {
            painter.drawImage(rect,compasImg);
            // draw the compass needle
            QPointF *points = new QPointF[3];
            points[0] = QPointF(-compasSize/5.0,0);
            points[1] = QPointF(compasSize/5.0,0);
            points[2] = QPointF(0,compasSize/2.2);
            painter.save();
            painter.translate(compasSize/2.0,compasSize/2.0);
            painter.rotate(azimut);
            painter.setBrush(Qt::white);
            painter.setPen(Qt::black);
            painter.drawPolygon(points,3);
            painter.setBrush(Qt::red);
            painter.rotate(180);
            painter.drawPolygon(points,3);
            painter.restore();
        }
    }
}

void Drone::resizeEvent(QResizeEvent *) {
    QRect rect(compasSize+5,0,width()-compasSize-5,compasSize/2);
    speedPB->setGeometry(rect);
    rect.setRect(compasSize+5,compasSize/2,width()-compasSize-5,compasSize/2);
    powerPB->setGeometry(rect);
}

void Drone::update(double dt) {
    if (status==landed) {
        power+=dt*chargingSpeed;
        if (power>maxPower) {
            power=maxPower;
        }
        powerPB->setValue(power);
        repaint();
        return;
    }

    if (status==takeoff) {
        height+=dt*takeoffSpeed;
        if (height>=hoveringHeight) {
            height=hoveringHeight;
            status=hovering;
        }
        power-=dt*powerConsumption;
        if (power<20+powerConsumption/takeoffSpeed) {
            status=landing;
            speed=0;
        }
        powerPB->setValue(power);
        repaint();
        return;
    }

    if (status==landing) {
        height-=dt*takeoffSpeed;
        if (height<=0) {
            height=0;
            status=landed;
            showCollision=false;
        }
        power-=dt*powerConsumption;
        powerPB->setValue(power);
        repaint();
        return;
    }

    if (status>=hovering) {
        Vector2D toGoal=goalPosition-position;
        double distance = toGoal.length();

        double damp= 1-dt*(1-damping);
        V = damp*V+((maxPower*dt/distance)*toGoal)+dt*ForceCollision;
        position += dt*V;
        speed=V.length();
        Vector2D Vn = (1.0/speed)*V;
        if (Vn.y==0) {
            if (Vn.x>0) {
                azimut = -90;
            } else {
                azimut = 90.0;
            }
        } else if (Vn.y>0) {
            azimut = 180.0-180.0*atan(Vn.x/Vn.y)/M_PI;
        } else {
            azimut = -180.0*atan(Vn.x/Vn.y)/M_PI;
        }
        if (toGoal.length()<1.0 && speed<10) {
            V.set(0,0);
            speed=0;
            status=landing;
        }
        speedPB->setValue(speed);
        power-=dt*powerConsumption;
        if (power<20+powerConsumption/takeoffSpeed) {
            speed=0;
            V.set(0,0);
            status=landing;
        }
        powerPB->setValue(power);
    }
    repaint();
}

void Drone::initCollision() {
    ForceCollision.set(0,0);
    showCollision=false;
}


void Drone::addCollision(const Vector2D& B,float threshold) {
    Vector2D AB=B-position;
    double l=AB.length();
    if (l<threshold) {
        ForceCollision+=(-coefCollision/threshold)*AB;
        showCollision=true;
    }
}
