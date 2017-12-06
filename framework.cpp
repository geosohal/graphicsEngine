///////////////////////////////////////////////////////////////////////
// Provides the framework for graphics projects.  Most of this small
// file contains the GLUT calls needed to open a window and hook up
// various callbacks for mouse/keyboard interaction and screen resizes
// and redisplays.
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////

#include "framework.h"

Scene scene;


////////////////////////////////////////////////////////////////////////
// Do the OpenGL/GLut setup and then enter the interactive loop.
int main(int argc, char** argv)
{
    // Initialize the OpenGL bindings
    glbinding::Binding::initialize(false);

    // Initialize freeglut and choose a version.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

    // Open a window
    glutInitWindowSize(600,600);
    glutCreateWindow("Class Framework");
    glutSetOption((GLenum)GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

    //printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    //printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    //printf("Rendered by: %s\n", glGetString(GL_RENDERER));
    //fflush(stdout);

    // Initialize interaction and the scene to be drawn.
    InitInteraction();
    scene.InitializeScene();

    // Enter the event loop.
    glutMainLoop();
}
