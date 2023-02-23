#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
#include <vecmath.h>
#include <algorithm>
#include <unistd.h>
using namespace std;

// Globals
const int MAX_BUFFER_SIZE = 1024;
int color_counter = 0;	    // The counter for H index of HSL color applied to the object.
float light_x_displ;	    // The displacement of x coordinate of the light source.
float light_y_displ;	    // The displacement of y coordinate of the light source.
float cam_phi_displ;        // The displacement of phi coordinate of the camera.
float cam_tht_displ;        // The displacement of theta coordinate of the camera.
float cam_zoom_factor = 1;  // The zoom factor.
bool rotation_status;       // The switch for rotation feature.
float rotation_angle;       // The degree for rotation.

// This is the list of points (3D vectors)
vector<Vector3f> vecv;

// This is the list of normals (also 3D vectors)
vector<Vector3f> vecn;

// This is the list of faces (indices into vecv and vecn)
vector<vector<unsigned>> vecf;

// You will need more global variables to implement color and position changes

// These are convenience functions which allow us to call OpenGL
// methods on Vec3d objects
/*
inline void glVertex(const Vector3f &a)
{ glVertex3fv(a); }

inline void glNormal(const Vector3f &a)
{ glNormal3fv(a); }
*/

// This function returns the rotation matrix on z-axis in a 3D space.
vector<Vector3f> rotationMatrix(float theta)
{
    vector<Vector3f> R(3);

    R[0][0] = cos(theta);
    R[0][2] = -sin(theta);
    R[2][0] = -R[0][2];
    R[2][2] = R[0][0];
    R[1][1] = 1;

    // for (int i = 0; i < 3; i++) {
    //     for (int j = 0; j < 3; j++) {
    //         cout << R[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    return R;
}

Vector3f mult(vector<Vector3f> M, Vector3f v)
{
    Vector3f result = {};

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            result[i] += M[i][j] * v[j];

    return result;
}

// This function converts three first element of input from RGB to HSL color format.
void HSL_to_RGB(GLfloat diffColor[4])
{
	// https://www.had2know.org/technology/hsl-rgb-color-converter.html

	GLfloat H = diffColor[0];
	GLfloat S = diffColor[1];
	GLfloat L = diffColor[2];

	GLfloat d = S * (1 - abs(2*L-1));
	GLfloat m = (L - d/2);
	GLfloat x = d * (1 - abs((H/60) - 2*(int(H)/120) - 1));

	if (H < 60) {
		diffColor[0] = d + m;	// Red
		diffColor[1] = x + m;	// Green
		diffColor[2] = m;		// Blue
	}
	else if (H < 120) {
		diffColor[0] = x + m;	// Red
		diffColor[1] = d + m;	// Green
		diffColor[2] = m;		// Blue
	}

	else if (H < 180) {
		diffColor[0] = m;		// Red
		diffColor[1] = d + m;	// Green
		diffColor[2] = x + m;	// Blue
	}
	else if (H < 240) {
		diffColor[0] = m;		// Red
		diffColor[1] = x + m;	// Green
		diffColor[2] = d + m;	// Blue
	}
	else if (H < 300) {
		diffColor[0] = x + m;	// Red
		diffColor[1] = m;		// Green
		diffColor[2] = d + m;	// Blue
	}
	else if (H < 360) {
		diffColor[0] = d + m;	// Red
		diffColor[1] = m;		// Green
		diffColor[2] = x + m;	// Blue
	}
}

// This function loads the OBJ model from standard input stream.
// The model is supposed to only have triangles.
void loadInput()
{
    // load the OBJ file here
    char buffer[MAX_BUFFER_SIZE];
    
    while (cin.getline(buffer, MAX_BUFFER_SIZE))
    {
        replace(buffer, buffer + MAX_BUFFER_SIZE, '/', ' ');
        stringstream ss(buffer);
        string token;
        ss >> token;

        if (token == "v")
        {
            Vector3f v;
            ss >> v[0] >> v[1] >> v[2];
            vecv.push_back(v);
        }
        else if (token == "vn")
        {
            Vector3f n;
            ss >> n[0] >> n[1] >> n[2];
            vecn.push_back(n);
        }
        else if (token == "f")
        {
            unsigned int a, b, c, d, e, f, g, h, i;
            ss >> a >> b >> c;
            ss >> d >> e >> f;
            ss >> g >> h >> i;

            // cout << "cin successfull: \n" << a << " " << b << " " << c << " " 
            //                               << d << " " << e << " " << f << " " 
            //                               << g << " " << h << " " << i << endl;
            
            vector<unsigned> face {a, b, c, d, e, f, g, h, i};
            vecf.push_back(face);
        }
    }
    
}

// This function simplifies calling gl of a vector vectex or normal.
inline void glNormal3d(Vector3f vec) { glNormal3d(vec[0], vec[1], vec[2]); }
inline void glVertex3d(Vector3f vec) { glVertex3d(vec[0], vec[1], vec[2]); }

// This function renders object based on triangle faces stored on vecf.
void renderObject()
{
    for (auto& face: vecf)
    {
        glBegin(GL_TRIANGLES);
        glNormal3d(vecn[face[2]-1]);
        glVertex3d(vecv[face[0]-1]);
        glNormal3d(vecn[face[5]-1]);
        glVertex3d(vecv[face[3]-1]);
        glNormal3d(vecn[face[8]-1]);
        glVertex3d(vecv[face[6]-1]);
        glEnd();
    }
}

// This function applies rotation on vertices and normals.
void rotationFunc(int value)
{
    if (rotation_status)
    {
        for (auto &v: vecv)
            v = mult(rotationMatrix(0.01), v);
        
        for (auto &n: vecn)
            n = mult(rotationMatrix(0.01), n);

        // this will refresh the screen so that the user sees the rotation change
        glutPostRedisplay();

        // cout << value << "\n";

        glutTimerFunc(20, rotationFunc, value + 1); // Apply rotation function
    }
}

// This function is called whenever a "Normal" key press is received.
void keyboardFunc(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // Escape key
        exit(0);
        break;
    case 'c':
        // code to change color here
        color_counter += 5;
        break;
    case 'r':
        // code to change rotation status here
        rotation_status = 1 - rotation_status;
        rotationFunc(0);
        break;
    case 'w':
        // up
        cam_phi_displ -= 0.05;
        break;
    case 's':
        // down
        cam_phi_displ += 0.05;
        break;
    case 'd':
        // right
        cam_tht_displ -= 0.05;
        break;
    case 'a':
        // left
        cam_tht_displ += 0.05;
        break;
    case 'z':
        // zoom-in
        cam_zoom_factor *= 0.9;
        break;
    case 'x':
        // zoom-out
        cam_zoom_factor /= 0.9;
        break;
    default:
        cout << "Unhandled key press " << key << "." << endl;
    }

    // this will refresh the screen so that the user sees the color change
    glutPostRedisplay();
}

// This function is called whenever a "Special" key press is received.
// Right now, it's handling the arrow keys.
void specialFunc(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        // add code to change light position
        light_y_displ += 0.5;
        // cout << "Unhandled key press: up arrow." << endl;
        break;
    case GLUT_KEY_DOWN:
        // add code to change light position
        light_y_displ -= 0.5;
        // cout << "Unhandled key press: down arrow." << endl;
        break;
    case GLUT_KEY_LEFT:
        // add code to change light position
        light_x_displ -= 0.5;
        // cout << "Unhandled key press: left arrow." << endl;
        break;
    case GLUT_KEY_RIGHT:
        // add code to change light position
        light_x_displ += 0.5;
        // cout << "Unhandled key press: right arrow." << endl;
        break;
    }

    // this will refresh the screen so that the user sees the light position
    glutPostRedisplay();
}

// This function is responsible for displaying the object.
void drawScene(void)
{
    int i;

    // Clear the rendering window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Rotate the image
    glMatrixMode(GL_MODELVIEW); // Current matrix affects objects positions
    glLoadIdentity();           // Initialize to the identity

    float r = 5.0 * cam_zoom_factor;
    float phi = M_PI_2 + cam_phi_displ;
    float theta = M_PI_2 + cam_tht_displ;

    float x = r * sin(phi) * cos(theta);
    float z = r * sin(phi) * sin(theta);
    float y = r * cos(phi);

    // Position the camera at [0,0,5], looking at [0,0,0],
    // with [0,1,0] as the up direction.
    gluLookAt(x, y, z,
              0.0, 0.0, 0.0,
              0.0, (sin(phi) > 0) ? 1.0 : -1.0, 0.0);

    // Set material properties of object

    // Here are some colors you might use - feel free to add more
    GLfloat diffColors[4][4] = {{0.5, 0.5, 0.9, 1.0},
                                {0.9, 0.5, 0.5, 1.0},
                                {0.5, 0.9, 0.3, 1.0},
                                {0.3, 0.8, 0.9, 1.0}};

	GLfloat diffColor[4] = {float(color_counter % 360), 0.8, 0.5, 1.0};
	HSL_to_RGB(diffColor);

    // Here we use the first color entry as the diffuse color
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffColor); //[color_counter % 4]);

    // Define specular color and shininess
    GLfloat specColor[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat shininess[] = {100.0};

    // Note that the specular color and shininess can stay constant
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    // Set light properties

    // Light color (RGBA)
    GLfloat Lt0diff[] = {1.0, 1.0, 1.0, 1.0};
    // Light position
    GLfloat Lt0pos[] = {1.0f + light_x_displ, 1.0f + light_y_displ, 5.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_DIFFUSE, Lt0diff);
    glLightfv(GL_LIGHT0, GL_POSITION, Lt0pos);

    // This GLUT method draws a teapot.  You should replace
    // it with code which draws the object you loaded.
    renderObject();
    // glutSolidTeapot(1.0);

    // Dump the image to the screen.
    glutSwapBuffers();
}

// Initialize OpenGL's rendering modes
void initRendering()
{
    glEnable(GL_DEPTH_TEST); // Depth testing must be turned on
    glEnable(GL_LIGHTING);   // Enable lighting calculations
    glEnable(GL_LIGHT0);     // Turn on light #0.
}

// Called when the window is resized
// w, h - width and height of the window in pixels.
void reshapeFunc(int w, int h)
{
    // Always use the largest square viewport possible
    if (w > h)
    {
        glViewport((w - h) / 2, 0, h, h);
    }
    else
    {
        glViewport(0, (h - w) / 2, w, w);
    }

    // Set up a perspective view, with square aspect ratio
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 50 degree fov, uniform aspect ratio, near = 1, far = 100
    gluPerspective(50.0, 1.0, 1.0, 100.0);
}

// Main routine.
// Set up OpenGL, define the callbacks and start the main loop
int main(int argc, char **argv)
{
    loadInput();

    glutInit(&argc, argv);

    // We're going to animate it, so double buffer
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Initial parameters for window position and size
    glutInitWindowPosition(60, 60);
    glutInitWindowSize(360, 360);
    glutCreateWindow("Assignment 0");

    // Initialize OpenGL parameters.
    initRendering();

    // Set up callback functions for key presses
    glutKeyboardFunc(keyboardFunc); // Handles "normal" ascii symbols
    glutSpecialFunc(specialFunc);   // Handles "special" keyboard keys

    // Set up the callback function for resizing windows
    glutReshapeFunc(reshapeFunc);

    // Call this whenever window needs redrawing
    glutDisplayFunc(drawScene);

    // Start the main loop.  glutMainLoop never returns.
    glutMainLoop();

    return 0; // This line is never reached.
}
