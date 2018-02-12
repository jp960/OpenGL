/*********************************************************************************************
	INCLUDES
*********************************************************************************************/
#define _USE_MATH_DEFINES
#include <math.h>       // For mathematic operations.
#ifdef __APPLE__
#include <OpenGL/gl.h>  // The GL header file.
#include <GLUT/glut.h>  // The GL Utility Toolkit (glut) header.
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include "objloader.hpp"
#include "cubeobjloader.hpp"
#include <GL/gl.h>      // The GL header file.
#include <GL/glut.h>       // The GL Utility Toolkit (glut) header (boundled with this program).
#endif

/*********************************************************************************************
	GLOBAL VARIABLES
*********************************************************************************************/

// Rendering mode.
char rendermode;
char renderobj;

// Angle of rotation
float rotAngle = (5.0f/180.0f) * M_PI;
	
// Object Booleans
bool loadBunny    = false;
bool loadCube     = false;
bool loadSD       = false;
bool loadElephant = false;

// Object vertices and indices
std::vector<std::array<float, 3>> vertices;
std::vector<std::array<int,   3>> triVertexIndices;
std::vector<std::array<int,   4>> quadVertexIndices;

// Camera
std::vector<std::array<float, 3>> camVectors;
std::array<float, 3> cam = { 2.8f, 3.4f, 7.7f };
std::array<float, 3> look = { 0.0f, 0.0f, 0.0f };
std::array<float, 3> right_left = { 1.0f, 0.0f, 0.0f };
std::array<float, 3> up_down = { 0.0f, 1.0f, 0.0f };
std::array<float, 3> forward_backward = { 0.0f, 0.0f, 1.0f };

// Texture
GLuint texture;

/*********************************************************************************************
	FUNCTIONS
*********************************************************************************************/

// Scene initialisation.
void InitGL(GLvoid)
{
	glShadeModel(GL_SMOOTH);               // Enable smooth shading.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Black background.
	glClearDepth(1.0f);                    // Depth buffer setup.
	glEnable(GL_DEPTH_TEST);               // Enables depth testing.
	glDepthFunc(GL_LEQUAL);                // The type of depth testing to do.
	glEnable(GL_COLOR_MATERIAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void idle(void)
{
	glutPostRedisplay();  // Trigger display callback.
}

/*********************************************************************************************
	VECTOR FUNCTIONS
*********************************************************************************************/
// Returns a float containing the length of the vector in the parameter.
float vectorLength(std::array<float, 3> v) {
	return sqrt((pow(v[0], 2.0f)) + (pow(v[1], 2.0f)) + (pow(v[2], 2.0f)));
}

std::array<float, 3> calcNormal(std::array<float, 3> v1, std::array<float, 3> v2, std::array<float, 3> v3) {
	// 2 arrays to hold the vectors a, b and the normal vector
	std::array<float, 3> a;
	std::array<float, 3> b;
	std::array<float, 3> normal;

	// a = v2 - v1, b = v3 - v1
	for (int i = 0; i < 3; i++) {
		a[i] = v2[i] - v1[i];
		b[i] = v3[i] - v1[i];
	}

	// Sets the normal coordinates to be the result of the cross product of a and b
	normal[0] = ((a[1] * b[2]) - (a[2] * b[1]));
	normal[1] = ((a[2] * b[0]) - (a[0] * b[2]));
	normal[2] = ((a[0] * b[1]) - (a[1] * b[0]));

	// Calculates the length of the cross product result
	float normal_length = vectorLength(normal);

	// The normal is the cross product result divided by the normal length
	normal[0] /= normal_length;
	normal[1] /= normal_length;
	normal[2] /= normal_length;

	return normal;
}

/*********************************************************************************************
	TRANSFORM FUNCTIONS
*********************************************************************************************/

// Takes a vector array and an integer of the direction as parameters
void rotateX(std::vector<std::array<float, 3>> &points, int direction) {
	// Creates a new variable of the angle of rotation * the direction (1/-1)
	float rotate = rotAngle * direction;
	// Iterates over the vector array
	for(int i = 0; i < points.size(); i++) {
		// Applies rotation to other two axes (Y and Z for X axis)
		// Stores the current values in new variables
		float y = points[i][1];
		float z = points[i][2];

		// Uses 3D rotation matrices to set the new values of those coordinates
		points[i][1] = y * cos(rotate) - z * sin(rotate);
		points[i][2] = y * sin(rotate) + z * cos(rotate);
	}
}

// Takes a vector array and an integer of the direction as parameters
void rotateY(std::vector<std::array<float, 3>> &points, int direction) {
	// Creates a new variable of the angle of rotation * the direction (1/-1)
	float rotate = rotAngle * direction;
	// Iterates over the vector array
	for(int i = 0; i < points.size(); i++) {
		// Applies rotation to other two axes (X and Z for Y axis)
		// Stores the current values in new variables
		float x = points[i][0];
		float z = points[i][2];

		// Uses 3D rotation matrices to set the new values of those coordinates
		points[i][0] = x * cos(rotate) + z * sin(rotate);
		points[i][2] = -x * sin(rotate) + z * cos(rotate);
	}
}

// Takes a vector array and an integer of the direction as parameters
void rotateZ(std::vector<std::array<float, 3>> &points, int direction) {
	// Creates a new variable of the angle of rotation * the direction (1/-1)
	float rotate = rotAngle * direction;
	// Iterates over the vector array
	for(int i = 0; i < points.size(); i++) {
		// Applies rotation to other two axes (X and Y for Z axis)
		// Stores the current values in new variables
		float x = points[i][0];
		float y = points[i][1];

		// Uses 3D rotation matrices to set the new values of those coordinates
		points[i][0] = x * cos(rotate) - y * sin(rotate);
		points[i][1] = x * sin(rotate) + y * cos(rotate);
	}
}

/*********************************************************************************************
	CAMERA
*********************************************************************************************/

// Sets the cameras initial coordinates and adds the 4 arrays into the array vector.
void camStartPos() {
	cam = { 4.8f, 3.4f, 7.7f };
	look = { 0.07f, 0.63f, 0.71f };
	camVectors.clear();
	camVectors.push_back(right_left);
	camVectors.push_back(up_down);
	camVectors.push_back(forward_backward);
	camVectors.push_back(look);
}

// Rotates the camera's by calling the rotate functions on the camVectors array
void rotateCam(int direction) {
	// Move Camera to origin
	for(int i = 0; i < 3; i++) {
		camVectors[3][i] = camVectors[3][i] - cam[i];
	}

	// Switch-case using the direction parameter to decide what direction
	// and what axis of rotation to use
	switch (direction) {
		case 1:
		{
			rotateX(camVectors, 1);
			break;
		}
		case 2:
		{
			rotateY(camVectors, 1);
			break;
		}
		case 3:
		{
			rotateZ(camVectors, 1);
			break;
		}
		case -1:
		{
			rotateX(camVectors, -1);
			break;
		}
		case -2:
		{
			rotateY(camVectors, -1);
			break;
		}
		case -3:
		{
			rotateZ(camVectors, -1);
			break;
		}
	}

	// Move Camera back to original position
	for(int i = 0; i < 3; i++) {
		camVectors[3][i] = camVectors[3][i] + cam[i];
	}
}

// Moves Camera position along line of sight and allows zoom (translate about z axis)
void translateCam(int direction) {
	// Variable to store length of direction vector
	float length;

	// Switch-case using the direction parameter to decide what direction
	// and what axis of rotation to use
	switch (direction) {
		// X +
		case 1:
		{
			// Find the length of the  direction vector
			length = vectorLength(camVectors[0]);
			// Iterate 3 times (3 coordinates) and increment by the normalised
			// direction vector to both cam and look (camVectors[3] = look)
			for (int i = 0; i < 3; i++) {
				cam[i] += camVectors[0][i] / length;
				camVectors[3][i] += camVectors[0][i] / length;
			}
			break;
		}

		// Y +
		case 2:
		{
			// Find the length of the  direction vector
			length = vectorLength(camVectors[1]);
			// Iterate 3 times (3 coordinates) and increment by the normalised
			// direction vector to both cam and look (camVectors[3] = look)
			for (int i = 0; i < 3; i++) {
				cam[i] += camVectors[1][i] / length;
				camVectors[3][i] += camVectors[1][i] / length;
			}
			break;
		}

		// Z +
		case 3:
		{
			// Find the length of the  direction vector
			length = vectorLength(camVectors[2]);
			// Iterate 3 times (3 coordinates) and increment by the normalised
			// direction vector to both cam and look (camVectors[3] = look)
			for (int i = 0; i < 3; i++) {
				cam[i] += camVectors[2][i] / length;
				camVectors[3][i] += camVectors[2][i] / length;
			}
			break;
		}

		// X -
		case -1:
		{
			// Find the length of the  direction vector
			length = vectorLength(camVectors[0]);
			// Iterate 3 times (3 coordinates) and decrement by the normalised
			// direction vector to both cam and look (camVectors[3] = look)
			for (int i = 0; i < 3; i++) {
				cam[i] -= camVectors[0][i] / length;
				camVectors[3][i] -= camVectors[0][i] / length;
			}
			break;
		}

		// Y -
		case -2:
		{
			// Find the length of the  direction vector
			length = vectorLength(camVectors[1]);
			// Iterate 3 times (3 coordinates) and decrement by the normalised
			// direction vector to both cam and look (camVectors[3] = look)
			for (int i = 0; i < 3; i++) {
				cam[i] -= camVectors[1][i] / length;
				camVectors[3][i] -= camVectors[1][i] / length;
			}
			break;
		}

		// Z -
		case -3:
		{
			// Find the length of the  direction vector
			length = vectorLength(camVectors[2]);
			// Iterate 3 times (3 coordinates) and decrement by the normalised
			// direction vector to both cam and look (camVectors[3] = look)
			for (int i = 0; i < 3; i++) {
				cam[i] -= camVectors[2][i] / length;
				camVectors[3][i] -= camVectors[2][i] / length;
			}
			break;
		}
	}
}

/*********************************************************************************************
	TEXTURE
*********************************************************************************************/

// Function copied from http://stackoverflow.com/questions/12518111/how-to-load-a-bmp-on-glut-to-use-it-as-a-texture
GLuint LoadTexture( const char * filename )
{
	int width, height;
	unsigned char * data;
	FILE * file;

	file = fopen( filename, "rb" );

	if ( file == NULL ) return 0;
	width = 256;
	height = 256;
	data = (unsigned char *)malloc( width * height * 3 );

	// Read the header of the bmp into a char array
	char header[54];
	fread( header, 54, 1, file);

	fread( data, width * height * 3, 1, file );
	fclose( file );

	for(int i = 0; i < width * height ; ++i)
	{
		int index = i*3;
		unsigned char B,R;
		B = data[index];
		R = data[index+2];

		data[index] = R;
		data[index+2] = B;
	}


	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST );


	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT );
	gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height,GL_RGB, GL_UNSIGNED_BYTE, data );
	free( data );

	return texture;
}

/*********************************************************************************************
	DRAW OBJECTS
*********************************************************************************************/
void draw_axes() {
	// X Axis
	// Sets the width of the axis line
	glLineWidth(1.0);
	// Sets the colour of the axis to Red
	glColor3f(1.0, 0.0, 0.0);
	// Sets the draw object as GL_LINES
	glBegin(GL_LINES);
	// Draws a line 200 pixels long (from -100 to 100)
	glVertex3f(-100.0, 0.0, 0.0);
	glVertex3f(100.0, 0.0, 0.0);
	glEnd();

	// Y Axis
	// Sets the width of the axis line
	glLineWidth(1.0);
	// Sets the colour of the axis to Green
	glColor3f(0.0, 1.0, 0.0);
	// Sets the draw object as GL_LINES
	glBegin(GL_LINES);
	// Draws a line 200 pixels long (from -100 to 100)
	glVertex3f(0.0, -100.0, 0.0);
	glVertex3f(0.0, 100.0, 0.0);
	glEnd();

	// Z Axis
	// Sets the width of the axis line
	glLineWidth(1.0);
	// Sets the colour of the axis to Blue
	glColor3f(0.0, 0.0, 1.0);
	// Sets the draw object as GL_LINES
	glBegin(GL_LINES);
	// Draws a line 200 pixels long (from -100 to 100)
	glVertex3f(0.0, 0.0, -100.0);
	glVertex3f(0.0, 0.0, 100.0);
	glEnd();
}

void draw_triangular_obj(bool load) {
	if (!load) {
		exit(39);
	}
	switch (rendermode) {
		case 'v':
		{
			// Draw points
			glBegin(GL_POINTS);
			// Iterates over the vertices array to get the vertex coordinates
			for (int i = 0; i < vertices.size(); i++) {
				// New variable to hold the current set of vertices
				std::array<float, 3> v = vertices[i];
				glColor3f(1.0f, 1.0f, 1.0f);
				// The vertex to be drawn using GL_POINTS
				glVertex3f(v[0], v[1], v[2]);
			}
			glEnd();
			// Sets the point size
			glPointSize(1);
			break;
		}

		case 'e':
		{
			// Draw lines
			glBegin(GL_LINES);
			int i = 0;
			// Iterates over the quadVertexIndices array to get each face
			while (i < triVertexIndices.size())
			{
				// Creates a new array to hold the vertex indices of the current face.
				std::array<int, 3> edge = triVertexIndices[i];
				// Uses the indices array to index the vertices array and get the coordinates
				// of each vertex
				std::array<float, 3> v1 = vertices[edge[0] - 1];
				std::array<float, 3> v2 = vertices[edge[1] - 1];
				std::array<float, 3> v3 = vertices[edge[2] - 1];

				glColor3f(1.0f, 0.0f, 1.0f);

				// Draws a line from vertex 1 to vertex 2
				glVertex3f(v1[0], v1[1], v1[2]);
				glVertex3f(v2[0], v2[1], v2[2]);

				// Draws a line from vertex 2 to vertex 3
				glVertex3f(v2[0], v2[1], v2[2]);
				glVertex3f(v3[0], v3[1], v3[2]);

				// Draws a line from vertex 1 to vertex 3
				glVertex3f(v1[0], v1[1], v1[2]);
				glVertex3f(v3[0], v3[1], v3[2]);
				i += 1;
			}
			glEnd();
			break;
		}

		case 'f':
		{
			// Enable Lighting
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);

			// Array to hold the normal of the current face
			std::array<float, 3> normal;
			// Triangles not quads
			glBegin(GL_TRIANGLES);
			int i = 0;

			// Iterates over the triVertexIndices array to get each face
			while (i < triVertexIndices.size()) {
				// Creates a new array to hold the vertex indices of the current face.
				std::array<int, 3> face = triVertexIndices[i];
				// Uses the indices array to index the vertices array and get the coordinates
				// of each vertex
				std::array<float, 3> v1 = vertices[face[0] - 1];
				std::array<float, 3> v2 = vertices[face[1] - 1];
				std::array<float, 3> v3 = vertices[face[2] - 1];

				// Sets the material colour to blue
				glColor3f(0.0f, 0.0f, 1.0f);
				// Sets the normal variable to be the result of the calcNormal function on
				// 3 adjacent vertices
				normal = calcNormal(v1, v2, v3);
				// Sets the coordinates of the normal variable as the normal of the current face
				glNormal3f(normal[0], normal[1], normal[2]);
				// Plots the 3 vertices of the face
				glVertex3f(v1[0], v1[1], v1[2]);
				glVertex3f(v2[0], v2[1], v2[2]);
				glVertex3f(v3[0], v3[1], v3[2]);
				i += 1;
			}
			glEnd();

			// Disable Lighting
			glDisable(GL_LIGHTING);
			glDisable(GL_LIGHT0);
			break;
		}
	}
}

void draw_quad_obj(bool load) {
	if (!load) {
		exit(39);
	}
	switch (rendermode) {
		case 'v':
		{
			// Draw points
			glBegin(GL_POINTS);
			// Iterates over the vertices array to get the vertex coordinates
			for (int i = 0; i < vertices.size(); i++) {
				// New variable to hold the current set of vertices
				std::array<float, 3> v = vertices[i];
				glColor3f(1.0f, 1.0f, 1.0f);
				// The vertex to be drawn using GL_POINTS
				glVertex3f(v[0], v[1], v[2]);
			}
			glEnd();
			// Sets the point size
			glPointSize(2);
			break;
		}

		case 'e':
		{
			// Draw lines
			glBegin(GL_LINES);
			int i = 0;
			// Iterates over the quadVertexIndices array to get each face
			while (i < quadVertexIndices.size())
			{
				// Creates a new array to hold the vertex indices of the current face.
				std::array<int, 4> edge = quadVertexIndices[i];
				// Uses the indices array to index the vertices array and get the coordinates
				// of each vertex
				std::array<float, 3> v1 = vertices[edge[0] - 1];
				std::array<float, 3> v2 = vertices[edge[1] - 1];
				std::array<float, 3> v3 = vertices[edge[2] - 1];
				std::array<float, 3> v4 = vertices[edge[3] - 1];

				glColor3f(1.0f, 0.0f, 1.0f);

				// Draws a line from vertex 1 to vertex 2
				glVertex3f(v1[0], v1[1], v1[2]);
				glVertex3f(v2[0], v2[1], v2[2]);

				// Draws a line from vertex 2 to vertex 3
				glVertex3f(v2[0], v2[1], v2[2]);
				glVertex3f(v3[0], v3[1], v3[2]);

				// Draws a line from vertex 3 to vertex 4
				glVertex3f(v3[0], v3[1], v3[2]);
				glVertex3f(v4[0], v4[1], v4[2]);

				// Draws a line from vertex 1 to vertex 4
				glVertex3f(v1[0], v1[1], v1[2]);
				glVertex3f(v4[0], v4[1], v4[2]);

				i += 1;
			}
			glEnd();
			break;
		}

		case 'f':
		{
			// An nested array that holds the texture coordinates of each face of the cube texture
			std::array<std::array<float, 8>, 6> cubeTexCoords;
			cubeTexCoords[0] = { 0.00f,  0.00f     , 0.25f,  0.00f     , 0.25f, (1.0f/3.0f), 0.00f, (1.0f/3.0f) };
			cubeTexCoords[1] = { 0.00f, (1.0f/3.0f), 0.25f, (1.0f/3.0f), 0.25f, (2.0f/3.0f), 0.00f, (2.0f/3.0f) };
			cubeTexCoords[2] = { 0.00f, (2.0f/3.0f), 0.25f, (2.0f/3.0f), 0.25f,  1.00f     , 0.00f,  1.00f      };
			cubeTexCoords[3] = { 0.25f, (1.0f/3.0f), 0.50f, (1.0f/3.0f), 0.50f, (2.0f/3.0f), 0.25f, (2.0f/3.0f) };
			cubeTexCoords[4] = { 0.50f, (1.0f/3.0f), 0.75f, (1.0f/3.0f), 0.75f, (2.0f/3.0f), 0.50f, (2.0f/3.0f) };
			cubeTexCoords[5] = { 0.75f, (1.0f/3.0f), 1.00f, (1.0f/3.0f), 1.00f, (2.0f/3.0f), 0.75f, (2.0f/3.0f) };

			// Enable Lighting and Textures
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
			glEnable(GL_TEXTURE_2D);

			// Array to hold the normal of the current face
			std::array<float, 3> normal;
			glBegin(GL_QUADS);
			int i = 0;
			// Iterates over the quadVertexIndices array to get each face
			while (i < quadVertexIndices.size()) {
				// Creates a new array to hold the vertex indices of the current face.
				std::array<int, 4> face = quadVertexIndices[i];
				// Uses the indices array to index the vertices array and get the coordinates
				// of each vertex
				std::array<float, 3> v1 = vertices[face[0] - 1];
				std::array<float, 3> v2 = vertices[face[1] - 1];
				std::array<float, 3> v3 = vertices[face[2] - 1];
				std::array<float, 3> v4 = vertices[face[3] - 1];

				// Sets the base colour as white
				glColor3f(1.0f, 1.0f, 1.0f);
				// Sets the normal variable to be the result of the calcNormal function on
				// 3 adjacent vertices
				normal = calcNormal(v1, v2, v3);
				// Sets the coordinates of the normal variable as the normal of the current face
				glNormal3f(normal[0], normal[1], normal[2]);

				// Uses the above texture coordinates for the cube and uses other coordinates for
				// the elephant.
				if (renderobj == '1') {
					// Sets the texture for each vertex of the cube faces
					glTexCoord2f(cubeTexCoords[i][0], cubeTexCoords[i][1]); glVertex3f(v1[0], v1[1], v1[2]);
					glTexCoord2f(cubeTexCoords[i][2], cubeTexCoords[i][3]); glVertex3f(v2[0], v2[1], v2[2]);
					glTexCoord2f(cubeTexCoords[i][4], cubeTexCoords[i][5]); glVertex3f(v3[0], v3[1], v3[2]);
					glTexCoord2f(cubeTexCoords[i][6], cubeTexCoords[i][7]); glVertex3f(v4[0], v4[1], v4[2]);
				}
				else {
					// Sets the texture for each vertex of the elephant faces
					glTexCoord2f(0.0f, 0.0f); glVertex3f(v1[0], v1[1], v1[2]);
					glTexCoord2f(0.0f, 1.0f); glVertex3f(v2[0], v2[1], v2[2]);
					glTexCoord2f(1.0f, 1.0f); glVertex3f(v3[0], v3[1], v3[2]);
					glTexCoord2f(1.0f, 0.0f); glVertex3f(v4[0], v4[1], v4[2]);
				}

				i += 1;
			}
			glEnd();

			// Disable Lighting and Textures for other objects/render modes
			glDisable(GL_LIGHTING);
			glDisable(GL_LIGHT0);
			glDisable(GL_TEXTURE_2D);
			break;
		}
	}
}

/*********************************************************************************************
	LOAD OBJECTS
*********************************************************************************************/
// Load Cube Object
void cube() {
	renderobj  = '1';
	// Clears the global vector arrays to hold new data
	vertices.clear();
	quadVertexIndices.clear();
	// Uses the cubeobjloader header to load the vertex data from the cube.obj file
	loadCube = load_cube_obj("cube3.obj", vertices, quadVertexIndices);
	// Loads the texture for the cube object
	texture =  LoadTexture( "dice.bmp" );
}

// Load Bunny Object
void bunny() {
	renderobj  = '2';
	// Clears the global vector arrays to hold new data
	vertices.clear();
	triVertexIndices.clear();
	// Uses the objloader header to load the vertex data from the bunny.obj file
	loadBunny = load_obj("bunny.obj", vertices, triVertexIndices);
}

// Load Screwdriver Object
void screwdriver() {
	renderobj  = '3';
	// Clears the global vector arrays to hold new data
	vertices.clear();
	triVertexIndices.clear();
	// Uses the objloader header to load the vertex data from the screwdriver.obj file
	loadSD = load_obj("screwdriver.obj", vertices, triVertexIndices);
}

// Load Elephant Object
void elephant() {
	renderobj  = '4';
	// Clears the global vector arrays to hold new data
	vertices.clear();
	quadVertexIndices.clear();
	// Uses the cubeobjloader header to load the vertex data from the elephant3.obj file
	loadElephant = load_cube_obj("elephant3.obj", vertices, quadVertexIndices);
	// Loads the texture for the elephant object
	texture =  LoadTexture( "yarn2.bmp" );
}

/*********************************************************************************************
	DISPLAY
*********************************************************************************************/
// Callback function that draws the requested objects
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	// Set the camera.
	gluLookAt(cam[0], cam[1], cam[2],
						camVectors[3][0], camVectors[3][1], camVectors[3][2],
						0.0f, 1.0f, 0.0f);

	// Lighting - default values for all properties
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position[] = { 0.0, 5.0, 5.0, 0.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	// Draw Cartesian coordinate system as lines
	draw_axes();

	// Different objects

	switch (renderobj) {
		case '1':
		{
			// Draw the cube using the draw_quad_obj function
			glPushMatrix();
			draw_quad_obj(loadCube);
			glPopMatrix();
			break;
		}

		case '2':
		{
			// Push top matrix of the stack, this is the matrix that represents the drawn object
			glPushMatrix();
			// Used OpenGL functions for scaling and translation
			glTranslatef(-0.5, 0.0, 0.0);
			glScalef(0.5, 0.5, 0.5);

			// Draw the bunny object using the draw_triangular_obj function
			draw_triangular_obj(loadBunny);

			// Pop the matrix back onto the stack
			glPopMatrix();
			break;
		}

		case '3':
		{
			// Push top matrix of the stack, this is the matrix that represents the drawn object
			glPushMatrix();
			// Used OpenGL functions for scaling and translation
			glTranslatef(-0.2, 4.0, 0.0);
			glScalef(1.6, 1.6, 1.6);

			// Draw the screwdriver object using the draw_triangular_obj function
			draw_triangular_obj(loadSD);

			// Pop the matrix back onto the stack
			glPopMatrix();
			break;
		}

		case '4':
		{
			// Draw the elephant object
			glPushMatrix();
			draw_quad_obj(loadElephant);
			glPopMatrix();
			break;
		}
	}
	glutSwapBuffers();
}



// The reshape function sets up the viewport and projection.
void reshape(int width, int height) {
	// Prevent a divide by zero error by making height equal to 1
	if (height == 0)
		height = 1;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Need to calculate the aspect ratio of the window for gluPerspective.
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	// Return to ModelView mode for future operations.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*********************************************************************************************
	USER INPUT
*********************************************************************************************/

// Callback for standard keyboard presses.
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		// Exit the program when escape is pressed
		case 27:
			exit(0);
			break;

		// Switch render mode.
		case 'v': rendermode = 'v'; break;  // vertices
		case 'e': rendermode = 'e'; break;  // edges
		case 'f': rendermode = 'f'; break;  // faces

		// Switch rendered object
		case '1': cube(); break;  // cube
		case '2': bunny(); break;  // bunny
		case '3': screwdriver(); break;  // screwdriver
		case '4': elephant(); break; // elephant

		// Rotate object positive
		case 'i': rotateY(vertices, 1); break; // Yaw Positive
		case 'o': rotateZ(vertices, 1); break; // Roll Positive
		case 'l': rotateX(vertices, 1); break; // Pitch Positive

		// Rotate object positive
		case 'k': rotateY(vertices, -1); break; // Yaw Negative
		case 'u': rotateZ(vertices, -1); break; // Roll Negative
		case 'j': rotateX(vertices, -1); break; // Pitch Negative

		// Rotates the camera viewport by 10 degrees about the Z axis when pressed
		case 'x': rotateCam(3); break;  
		case 'z': rotateCam(-3); break; 

		// Translates camera and viewport
		// Translating X axis
		case 'd': translateCam(1); break;  // left
		case 'a': translateCam(-1); break; // right

		// Translating Y axis
		case 'w': translateCam(2); break;  // up
		case 's': translateCam(-2); break; // down

		// Translating Z axis
		case '-': translateCam(3); break;  // forwards zoom
		case '=': translateCam(-3); break; // backwards zoom

		case 'b': camStartPos(); break;


	default:
		break;
	}

	glutPostRedisplay();
}

// Arrow keys need to be handled in a separate function from other keyboard presses.
void arrow_keys(int a_keys, int x, int y) {
	switch (a_keys) {
	case GLUT_KEY_UP:
		// Rotates the camera viewport by 10 degrees about Y axis when pressed
		rotateCam(1);
		break;

	case GLUT_KEY_DOWN:
		// Rotates the camera viewport by 10 degrees about Y axis when pressed
		rotateCam(-1);
		break;

	case GLUT_KEY_RIGHT:
		// Rotates the camera viewport by 10 degrees about X axis when pressed
		rotateCam(2);
		break;

	case GLUT_KEY_LEFT:
		// Rotates the camera viewport by 10 degrees about X axis when pressed
		rotateCam(-2);
		break;

	default:
		break;
	}

	glutPostRedisplay();
}



// Handling mouse button event.
void mouseButton(int button, int state, int x, int y)
{
}


// Handling mouse move events.
void mouseMove(int x, int y)
{
}


// Note: You may wish to add interactivity like clicking and dragging to move the camera.
//       In that case, please use the above functions.

/*********************************************************************************************
	MAIN FUNCTION
*********************************************************************************************/

// Entry point to the application.
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutInitWindowSize(500, 500);
	glutCreateWindow("CM20219 OpenGL Coursework");
	//glutFullScreen();  // Uncomment to start in full screen.
	InitGL();
	rendermode = 'v';

	camStartPos(); // Sets the camera's initial position coordinates
	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(arrow_keys);  // For special keys
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
	glutIdleFunc(idle);

	glutMainLoop();
}
