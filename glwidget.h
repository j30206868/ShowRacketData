#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QMouseEvent>
#include <QGLWidget>
#include <QTimer>

#include "math_3d.h"

class glwidget : public QGLWidget
{
    Q_OBJECT
public:
    QTimer timer;

    explicit glwidget(QWidget *parent = 0);

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void setNewZ(float x, float y, float z);
signals:

public slots:

protected:
    void mousePressEvent(QMouseEvent *event);
private:
    Vector3f origin_z;
    Vector3f new_z;
    float rotate_angle;
    //camera control
    float cameraRadius;
    float cameraAngleInDegree;
    float cameraX;
    float cameraY;
};

#endif // GLWIDGET_H
