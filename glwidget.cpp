#include "glwidget.h"
#include "GL/glut.h"

#include "math.h"
#include <iostream>

glwidget::glwidget(QWidget *parent) :
    QGLWidget(parent)
{
    //connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    //timer.start(16);
    origin_z = Vector3f(0, 0, -1);
}

void glwidget::initializeGL()
{
    glClearColor(0.2, 0.2, 0.2, 1);
    glEnable(GL_DEPTH_TEST);
    glShadeModel (GL_SMOOTH);
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHTING);
}

float RadianToDegree(float radian){
    return radian * 57.2958;
}
Vector3f CrossProduct(const Vector3f& v1, const Vector3f& v2)
{
    const float _x = v1.y * v2.z - v1.z * v2.y;
    const float _y = v1.z * v2.x - v1.x * v2.z;
    const float _z = v1.x * v2.y - v1.y * v2.x;

    return Vector3f(_x, _y, _z);
}
float DotProduct (const Vector3f& v1, const Vector3f& v2)
{
    return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}
float VectorMagnitude(Vector3f& v){
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}
Vector3f& NormalizeVector(Vector3f& v)
{
    const float Length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

    v.x /= Length;
    v.y /= Length;
    v.z /= Length;

    return v;
}
void glwidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //original z
    //origin_z.x = 0;
    //origin_z.y = 0;
    //origin_z.z = -1;
    //new z
    new_z.x    = -0.7071;
    new_z.y    =  0     ;
    new_z.z    = -0.7071;
    //外積結果
    origin_z = NormalizeVector(origin_z);
    new_z    = NormalizeVector(new_z);
    Vector3f rotate_axis  = CrossProduct(origin_z, new_z);

    rotate_angle = acos(DotProduct(origin_z, new_z) / (VectorMagnitude(origin_z) * VectorMagnitude(new_z)) );
    rotate_angle = RadianToDegree(rotate_angle);
    std::cout << "rotate_angle: " << rotate_angle << std::endl;

    glLoadIdentity();
    gluLookAt(0, -5, 0, 0, 0, 0, 0, 0, 1);
    //glTranslatef(0.0, 0.1, -1.0);
    glRotatef(rotate_angle, rotate_axis.x, rotate_axis.y ,rotate_axis.z);
    glBegin(GL_TRIANGLES);
        glColor3f(0, 1.0, 0);
        glVertex3f(-0.5, 0, 0 );
        glColor3f(0, 1.0, 0);
        glVertex3f( 0.5, 0, 0 );
        glColor3f(0, 0, 1.0);
        glVertex3f( origin_z.x, origin_z.y, origin_z.z);
    glEnd();
}

void glwidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)w/h, 0.01, 100.0);
    glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //gluLookAt(0, 0, 20, 0, 0, 0, 0, 1, 0);
}

void glwidget::mousePressEvent(QMouseEvent *event){
    std::cout << "release." << std::endl;
    updateGL();
}
