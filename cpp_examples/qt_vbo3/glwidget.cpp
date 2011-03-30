#include <iostream>

#include <GL/glut.h>

#include <QObject>
#include <QTimer>

#include "glwidget.h"

GLWidget::GLWidget( QWidget *parent, char *name ) 
  : QGLWidget(parent) {
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect( timer, SIGNAL(timeout()), this, SLOT(timeOutSlot()) );
    timer->start();
    fov_y = 60.0;
    z_near = 0.1;
    z_far = 100.0;
    up.x=0;up.y=1;up.z=1;
    eye.y=4;
    center.x=10;
    //setCursor(cursor);
}

GLData* GLWidget::addObject() {
    GLData* g = new GLData();
    glObjects.push_back(g);
    return g;
}

void GLWidget::initializeGL() {
    std::cout << "initializeGL()\n";
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glLoadIdentity();
    // glEnable(GL_DEPTH_TEST);
    genVBO();  
}

void GLWidget::resizeGL( int width, int height ) {
    std::cout << "resizeGL(" << width << " , " << height << " )\n";
    if (height == 0)    {
       height = 1;
    }
    glViewport(0, 0, width, height); // Reset The Current Viewport
    _width = width;
    _height = height;
    glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
    glLoadIdentity(); // Reset The Projection Matrix
    
    // Calculate The Aspect Ratio Of The Window
    // void gluPerspective( fovy, aspect, zNear, zFar);
    gluPerspective( fov_y, (GLfloat)width / (GLfloat)height, z_near, z_far);
    
    glMatrixMode(GL_MODELVIEW); // Select The Modelview Matrix
    glLoadIdentity(); // Reset The Modelview Matrix
    return;
}

void GLWidget::paintGL()  {
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt( eye.x, eye.y, eye.z, center.x, center.y, center.z, up.x, up.y, up.z );
    //glPushMatrix();
    
    BOOST_FOREACH( GLData* g, glObjects ) { // draw each object
        glLoadIdentity();
        glTranslatef( g->pos.x, g->pos.y , g->pos.z ); 
        
        if ( !g->bind() )
            assert(0);
        
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        
        // coords/vert, type, stride, pointer/offset
        glVertexPointer(3, GLData::coordinate_type, sizeof( GLData::vertex_type ), BUFFER_OFFSET(GLData::vertex_offset));
        glColorPointer(3, GLData::coordinate_type, sizeof( GLData::vertex_type ), BUFFER_OFFSET(GLData::color_offset)); // color is offset 12-bytes from position
        
        //              mode       idx-count     type         indices*/offset
        glDrawElements( g->type , g->indexCount() , GLData::index_type, 0);
         
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        
        g->release();
    }
    //glPopMatrix();
}


// mouse movement
// dx in [-1,1]
// dx = (xnew - xold) / width
// dy = (ynew - yold) / height

// function that rotates point around axis:
// rotatePoint( point, origin, direction, angle)

// rotation around axis: 
// dir_x = up
// dir_y = up cross (center-eye) / norm( up cross (center-eye) )
// now rotations are:
// eye = rotatePoint(eye, center, dir_x, -dx * pi )
// eye = rotatePoint(eye, center, dir_y, dy*pi )
// up = rotatePoint( center+up, center, dir_y, dy*pi) - center
// (normalize up after operations)
//
// zoom
// eye = center + (eye-center)*(dy+1)
//
// pan
// height of window in object space: length = 2* norm(eye-center)*tan(fovy/2)
// new position of center is:
// center = center + dir_y *dx*length *width/height
//                 + dir_x * dy * length



