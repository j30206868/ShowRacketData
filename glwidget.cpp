#include "glwidget.h"
#include "GL/glut.h"

#include "math.h"
#include <iostream>

static float cos_theta[361];
static float sin_theta[361];
void init_sin_cos_theta(){
    for(int i=0 ; i<=360 ; i++){
        sin_theta[i] = sin(i * 0.017453292);
        cos_theta[i] = cos(i * 0.017453292);
    }
}

glwidget::glwidget(QWidget *parent) :
    QGLWidget(parent)
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer.start(10);
    //原本指向的地方很重要, 因為angle轉到180的地方會翻面, 不好看
    origin_z = Vector3f(0       , 0        , 1);
    new_z    = Vector3f(0.202657, -0.376243, 0.903948);
    init_sin_cos_theta();

    cameraRadius = 10;
    cameraAngleInDegree = 180;
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

inline float RadianToDegree(float radian){
    return radian * 57.2958;
}
inline float DegreeToRadian(float degree){
    return degree / 57.2958;
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
void draw_XZ_circle(float r, float bx, float by, float bz){
    float x, y;
    float nx, ny;
    nx = cos_theta[0] * r;
    ny = sin_theta[0] * r;
    for(int i=0 ; i<360 ; i++){
        x = nx;
        y = ny;
        nx = cos_theta[i+1] * r;
        ny = sin_theta[i+1] * r;
        glColor3f(0, 0, 1.0);//拍面中心的顏色
        glVertex3f(bx+0.0f, by+0.0f, bz+0.0f);
        glColor3f(1.0f,1.0f,1.0f);
        glVertex3f(bx+x, by, bz+y);
        glColor3f(1.0f,1.0f,1.0f);
        glVertex3f(bx+nx, by, bz+ny);
    }
}
void draw_XY_circle(float r, float bx, float by, float bz){
    float x, y;
    float nx, ny;
    nx = cos_theta[0] * r;
    ny = sin_theta[0] * r;
    for(int i=0 ; i<360 ; i++){
        x = nx;
        y = ny;
        nx = cos_theta[i+1] * r;
        ny = sin_theta[i+1] * r;
        //glColor3f(1.0f,1.0f,1.0f);
        glVertex3f(bx+0.0f, by+0.0f, bz+0.0f);
        //glColor3f(1.0f,1.0f,1.0f);
        glVertex3f(bx+x, by+y, bz);
        //glColor3f(1.0f,1.0f,1.0f);
        glVertex3f(bx+nx, by+ny, bz);
    }
}
void connect_two_XY_circles(float r1, float bx1, float by1, float bz1,
                      float r2, float bx2, float by2, float bz2){
    float x1, y1, x2, y2;
    float nx1, ny1, nx2, ny2;
    nx1 = cos_theta[0] * r1;
    ny1 = sin_theta[0] * r1;
    nx2 = cos_theta[0] * r2;
    ny2 = sin_theta[0] * r2;
    for(int i=0 ; i<360 ; i++){
        x1 = nx1;
        y1 = ny1;
        x2 = nx2;
        y2 = ny2;
        nx1 = cos_theta[i+1] * r1;
        ny1 = sin_theta[i+1] * r1;
        nx2 = cos_theta[i+1] * r2;
        ny2 = sin_theta[i+1] * r2;
        //glColor3f(1.0f,0.0f,0.0f);
        glVertex3f(bx1+x1, by1+y1, bz1);
        //glColor3f(1.0f,0.0f,0.0f);
        glVertex3f(bx2+x2, by2+y2, bz2);
        //glColor3f(1.0f,0.0f,0.0f);
        glVertex3f(bx2+nx2, by2+ny2, bz2);
        //glColor3f(1.0f,0.0f,0.0f);
        glVertex3f(bx2+nx2, by2+ny2, bz2);
        //glColor3f(1.0f,0.0f,0.0f);
        glVertex3f(bx1+nx1, by1+ny1, bz1);
        //glColor3f(1.0f,0.0f,0.0f);
        glVertex3f(bx1+x1, by1+y1, bz1);
    }
}

void glwidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SHORT leftKeyState  = false;
    SHORT rightKeyState = false;
    SHORT downKeyState  = false;
    SHORT upKeyState    = false;

    bool cameraPosChanged = false;
    if( ( 1 << 16 ) & leftKeyState ){
        cameraAngleInDegree += 5;
        cameraPosChanged = true;
    }
    if( ( 1 << 16 ) & rightKeyState ){
        cameraAngleInDegree -= 5;
        cameraPosChanged = true;
    }
    if( ( 1 << 16 ) & downKeyState ){
        cameraRadius += 0.5;
        cameraPosChanged = true;
    }
    if( ( 1 << 16 ) & upKeyState ){
        cameraRadius -= 0.5;
        cameraPosChanged = true;
    }

    float radian_angle = DegreeToRadian(cameraAngleInDegree);
    cameraX      = cameraRadius * sin(radian_angle);
    cameraY      = cameraRadius * cos(radian_angle);

    if( cameraPosChanged ){
        std::cout << "CameraCenter(" << cameraX << ", " << cameraY << ") Radius: " << cameraRadius << std::endl;
    }

    //original z
    //origin_z.x = 0;
    //origin_z.y = 0;
    //origin_z.z = -1;
    //new z
    //new_z.x    =  0.202657;
    //new_z.y    = -0.376243;
    //new_z.z    =  0.903948;

    //外積結果
    origin_z = NormalizeVector(origin_z);
    new_z    = NormalizeVector(new_z);
    Vector3f rotate_axis  = CrossProduct(origin_z, new_z);

    rotate_angle = acos(DotProduct(origin_z, new_z) / (VectorMagnitude(origin_z) * VectorMagnitude(new_z)) );
    rotate_angle = RadianToDegree(rotate_angle);

    //std::cout << "new_z: " << new_z.x << " , " << new_z.y << " , " << new_z.z << ")" << std::endl;
    //std::cout << "rotate_angle: " << rotate_angle << std::endl;

    glLoadIdentity();
    gluLookAt(cameraX, cameraY, -3, 0, 0, 0, 0, 0, 1);
    glRotatef(rotate_angle, rotate_axis.x, rotate_axis.y ,rotate_axis.z);
    glBegin(GL_TRIANGLES);
        //Sensor部分
        glColor3f(1.0, 0, 0);
        connect_two_XY_circles(0.01, -0.05, 0, -0.15, 0.01, -0.05, 0, 0);
        glColor3f(0, 0, 0);
        connect_two_XY_circles(0.01, -0.04, 0, -0.15, 0.01, -0.04, 0, 0);
        glColor3f(0.3, 0, 0.3);
        connect_two_XY_circles(0.01, -0.03, 0, -0.15, 0.01, -0.03, 0, 0);
        //握把
        glColor3f(0, 1.0, 0);
        draw_XY_circle (0.1, 0, 0, 0);
        connect_two_XY_circles(0.1, 0, 0, 0, 0.1, 0, 0, 1);
        draw_XY_circle (0.1, 0, 0, 1);
        //連接握把與中軸
        glColor3f(0, 0, 1.0);
        connect_two_XY_circles(0.1 , 0, 0, 1  , 0.05, 0, 0, 1.2);
        //中軸
        glColor3f(1.0, 1.0, 1.0);
        draw_XY_circle (0.05, 0, 0, 1.2);
        connect_two_XY_circles(0.05, 0, 0, 1.2, 0.05, 0, 0, 2.4);
        draw_XY_circle (0.05, 0, 0, 2.4);
        //拍面
        glColor3f(1.0, 1.0, 1.0);
        draw_XZ_circle(0.6, 0, 0, 3);
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

void glwidget::setNewZ(float x, float y, float z)
{
    this->new_z.x = x;
    this->new_z.y = y;
    this->new_z.z = z;
}

void glwidget::mousePressEvent(QMouseEvent *event){
    std::cout << "press." << std::endl;
    updateGL();
}
