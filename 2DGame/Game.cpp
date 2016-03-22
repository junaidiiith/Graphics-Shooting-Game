#include <iostream>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
    bool obs;
    GLfloat x_centre;
    GLfloat y_centre;

    GLfloat radius;
    bool scorable;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;
    

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir;
float rectangle_rot_dir;
bool rectangle_rot_status;
bool triangle_rot_status = true;
float triangle_translationX;
float triangle_translationY;
VAO* Obstacles[100];
int add = 0;
float zoom = 1;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_Z:
              if(zoom < 1)
              {
                zoom = zoom + 0.005;
                Matrices.projection = glm::ortho(-zoom*4.0f, zoom*4.0f, -zoom*4.0f, zoom*4.0f, 0.1f, 500.0f);
              }
              break;
            case GLFW_KEY_X:
              if(zoom > 0.005)
              {
                zoom -= 0.005;
                Matrices.projection = glm::ortho(-zoom*4.0f, zoom*4.0f, -zoom*4.0f, zoom*4.0f, 0.1f, 500.0f);
              }
              break;
            default:
                break;
        }
    }
}

int space, fl;
/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
    case ' ':
            space=1;
            break;
    case 'a':
            rectangle_rot_dir=1;
            rectangle_rot_status=true;
            break;
    case 'd':
            rectangle_rot_dir=-1;
            rectangle_rot_status=true;
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-zoom*4.0f, zoom*4.0f, -zoom*4.0f, zoom*4.0f, 0.1f, 500.0f);
}

VAO *triangle,*circle1, *circle2,  *circle3, *circle4,*circle5, *rectangle, *cannon, *circle6, *circle7,*circle8;
VAO *rectangle1, *rectangle2, *rectangle3,*rectangle4, *rectangle5,*rectangle6, *rectangle7;
int countt;

// Creates the triangle object used in this sample code
// void createTriangle1 (GLfloat x, GLfloat y, GLfloat z,GLfloat radius,bool obs, bool scorable,int r, int g, int b)
// {
//   /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

//   /* Define vertex array as used in glBegin (GL_TRIANGLES) */
//   static const GLfloat vertex_buffer_data [] = {
//     x, y+radius,0, // vertex 0
//     x-(sqrt(3)/2)*radius,y-(radius/2),0, // vertex 1
//     x+(sqrt(3)/2)*radius,y-(radius/2),0, // vertex 2
//   };

//   // create3DObject creates and returns a handle to a VAO that can be used later
//   triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data,r,g,b);
  
//   triangle->obs = obs;
//   triangle->radius = radius;
//   triangle->x_centre = x;
//   triangle->y_centre = y;
//   triangle->scorable = scorable;
//   if(obs)
//     Obstacles[add++] = triangle;
// }


void drawCannon( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides,bool obs , bool scorable,int r,int g,int b)
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = x;
  circleVerticesY[0] = y;
  circleVerticesZ[0] = z;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = z;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

    // glEnableClientState( GL_VERTEX_ARRAY );
    //     glVertexPointer( 3, GL_FLOAT, 0, allCircleVertices );
    //     glDrawArrays( GL_TRIANGLE_FAN, 0, numberOfVertices);
    // glDisableClientState( GL_VERTEX_ARRAY );
  cannon = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,r,g,b);
  
  cannon->obs = obs;
  cannon->radius = radius;
  cannon->x_centre = x;
  cannon->y_centre = y;
}


void drawCircle1( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides,bool obs , bool scorable,int r,int g,int b)
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = x;
  circleVerticesY[0] = y;
  circleVerticesZ[0] = z;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = z;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

    // glEnableClientState( GL_VERTEX_ARRAY );
    //     glVertexPointer( 3, GL_FLOAT, 0, allCircleVertices );
    //     glDrawArrays( GL_TRIANGLE_FAN, 0, numberOfVertices);
    // glDisableClientState( GL_VERTEX_ARRAY );
  circle1 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,r,g,b);
  
  circle1->obs = obs;
  circle1->radius = radius;
  circle1->x_centre = x;
  circle1->y_centre = y;
  if(obs)
    Obstacles[add++] = circle1;
    Obstacles[add-1]->scorable = scorable;
}


void drawCircle2( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides,bool obs , bool scorable,int r,int g,int b)
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = x;
  circleVerticesY[0] = y;
  circleVerticesZ[0] = z;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = z;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

  circle2 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,r,g,b);
  
  circle2->obs = obs;
  circle2->radius = radius;
  circle2->x_centre = x;
  circle2->y_centre = y;
  if(obs)
    Obstacles[add++] = circle2;
    Obstacles[add-1]->scorable = scorable;
}


void drawCircle6( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides,bool obs , bool scorable,int r,int g,int b)
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = x;
  circleVerticesY[0] = y;
  circleVerticesZ[0] = z;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = z;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

  circle6 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,r,g,b);
  
  circle6->obs = obs;
  circle6->radius = radius;
  circle6->x_centre = x;
  circle6->y_centre = y;
  if(obs)
    Obstacles[add++] = circle6;
    Obstacles[add-1]->scorable = scorable;
}


void drawCircle7( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides,bool obs , bool scorable,int r,int g,int b)
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = x;
  circleVerticesY[0] = y;
  circleVerticesZ[0] = z;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = z;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

  circle7 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,r,g,b);
  
  circle7->obs = obs;
  circle7->radius = radius;
  circle7->x_centre = x;
  circle7->y_centre = y;
  if(obs)
    Obstacles[add++] = circle7;
    Obstacles[add-1]->scorable = scorable;
}

void drawCircle8( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides,bool obs , bool scorable,int r,int g,int b)
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = x;
  circleVerticesY[0] = y;
  circleVerticesZ[0] = z;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = z;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

  circle8 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,r,g,b);
  
  circle8->obs = obs;
  circle8->radius = radius;
  circle8->x_centre = x;
  circle8->y_centre = y;
  if(obs)
    Obstacles[add++] = circle8;
    Obstacles[add-1]->scorable = scorable;
}


void drawCircle4( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides,bool obs , bool scorable,int r,int g,int b)
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = x;
  circleVerticesY[0] = y;
  circleVerticesZ[0] = z;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = z;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

  circle4 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,r,g,b);
  
  circle4->obs = obs;
  circle4->radius = radius;
  circle4->x_centre = x;
  circle4->y_centre = y;
  if(obs)
    Obstacles[add++] = circle4;
    Obstacles[add-1]->scorable = scorable;
}


void drawCircle5( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides,bool obs , bool scorable,int r,int g,int b)
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = x;
  circleVerticesY[0] = y;
  circleVerticesZ[0] = z;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = z;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

  circle5 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,r,g,b);
  
  circle5->obs = obs;
  circle5->radius = radius;
  circle5->x_centre = x;
  circle5->y_centre = y;
  if(obs)
    Obstacles[add++] = circle2;
    Obstacles[add-1]->scorable = scorable;
}


void drawCircle3( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides,bool obs , bool scorable,int r,int g,int b)
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = x;
  circleVerticesY[0] = y;
  circleVerticesZ[0] = z;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = z;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

  circle3 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,r,g,b);
  
  circle3->obs = obs;
  circle3->radius = radius;
  circle3->x_centre = x;
  circle3->y_centre = y;
  if(obs)
    Obstacles[add++] = circle3;
    Obstacles[add-1]->scorable = scorable;
}



//Creates the rectangle object used in this sample code
void createRectangle (GLfloat x, GLfloat y, GLfloat z, GLfloat radius, bool obs, GLfloat angle)
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1
    x + radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 2
    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3

    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3
    x - radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 4
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1

  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  rectangle->obs = obs;
  rectangle->x_centre = x;
  rectangle->y_centre = y;
  rectangle->radius = radius;
  if(obs)
    Obstacles[add++] = rectangle;
}


void createRectangle1 (GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLfloat angle)
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1
    x + radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 2
    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3

    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3
    x - radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 4
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1

  };


  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, 0.5,0.2,0.5);
}



void createRectangle2 (GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLfloat angle)
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1
    x + radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 2
    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3

    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3
    x - radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 4
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1

  };

  static const GLfloat color_buffer_data [] = {
    1,0.84,0, // color 1
    1,0.84,0, // color 2
    1,0.84,0, // color 3

    1,0.84,0, // color 3
    1,0.84,0, // color 4
    1,0.84,0,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createRectangle3 (GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLfloat angle)
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1
    x + radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 2
    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3

    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3
    x - radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 4
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1

  };

  static const GLfloat color_buffer_data [] = {
    1,0.84,0, // color 1
    1,0.84,0, // color 2
    1,0.84,0, // color 3

    1,0.84,0, // color 3
    1,0.84,0, // color 4
    1,0.84,0,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRectangle4 (GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLfloat angle)
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1
    x + radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 2
    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3

    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3
    x - radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 4
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1

  };

  static const GLfloat color_buffer_data [] = {
    1,0.84,0, // color 1
    1,0.84,0, // color 2
    1,0.84,0, // color 3

    1,0.84,0, // color 3
    1,0.84,0, // color 4
    1,0.84,0,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle4 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRectangle5 (GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLfloat angle)
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1
    x + radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 2
    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3

    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3
    x - radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 4
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1

  };

  static const GLfloat color_buffer_data [] = {
    1,0.84,0, // color 1
    1,0.84,0, // color 2
    1,0.84,0, // color 3

    1,0.84,0, // color 3
    1,0.84,0, // color 4
    1,0.84,0,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle5 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createRectangle6 (GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLfloat angle)
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1
    x + radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 2
    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3

    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3
    x - radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 4
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1

  };


  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle6 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, 1,1,1);
}

void createRectangle7 (GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLfloat angle)
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1
    x + radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 2
    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3

    x + radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 3
    x - radius*cos(angle*(M_PI/180)),y - radius*sin(angle*(M_PI/180)),0, // vertex 4
    x - radius*cos(angle*(M_PI/180)),y + radius*sin(angle*(M_PI/180)),0, // vertex 1

  };


  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle7 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, 1,0,0);
}

// void createBar()

float camera_rotation_angle = 90;


/* Render the scene with openGL */
/* Edit this function according to your assignment */
double u,t;
GLfloat x_c,y_c,sp_x, sp_y, tspeed , yspeed, xspeed, angle , e,sx,fla ;
GLfloat g , flag;
GLfloat rectangle_rotation, triangle_rotation;
int score;
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;  // MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangle2);

  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangle3);

  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangle4);

  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangle5);

  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangle6);


  // Matrices.model = glm::mat4(1.0f);
  // MVP = VP * Matrices.model;
  // glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  // draw3DObject(triangle);

  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(circle4);

  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(circle5);


  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangle7 = glm::translate (glm::vec3(sx ,0.0 , 0));
  Matrices.model *= translateRectangle7;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangle7);
  if(!space)
  {
    if(sx >= 1.0 || sx <= -1.0)
      fla*= -1;
    sx+= fla*0.01;
 //   cout << sx << endl;
  }
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangle = glm::translate (glm::vec3(2.9,2.4, 0));
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 invtranslateRectangle = glm::translate (glm::vec3(-2.9, -2.4, 0));

  glm::mat4 rectangleTransform = invtranslateRectangle*rotateRectangle*translateRectangle;
  Matrices.model *= rectangleTransform; 

  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangle1);

  float increments = 1;
  if(rectangle_rot_status==true)
  {
    if(rectangle_rot_dir==1 && rectangle_rotation<=75)
    rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
    if(rectangle_rot_dir==-1 && rectangle_rotation>=10)
      rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
    rectangle_rot_status=false;
    angle=rectangle_rotation*M_PI/180.0f;
    triangle_rotation = rectangle_rotation;
  }


  int k = add;
  while(k--)
  {
    if(Obstacles[k]->obs)
    {  
      GLfloat r = Obstacles[k]->radius;
      GLfloat dis = cannon->radius + r;
      GLfloat xc = Obstacles[k]->x_centre;
      GLfloat yc = Obstacles[k]->y_centre;
      GLfloat d = sqrt((x_c - xc)*(x_c - xc) + (y_c - yc)*(y_c - yc));
      if(d <= dis)
      { 
        score += 5;
        cout << "Score = " << score << endl;
        Obstacles[k]->obs = false;
        if(triangle_translationY<0)
          u = sqrt(-4*triangle_translationY);
        else
          u = sqrt(4*triangle_translationY);
        // tspeed -= 0.01;
        angle = M_PI - atan((yspeed + g*t)/xspeed);
        sp_x = x_c;
        sp_y = y_c;
       // cout << "u =" << u << " angle = " << angle << " x = " << sp_x << " y = " << sp_y << " uy = " << u*sin(angle) << "\n";
        t = 0.08;
        break;
      }
      if(Obstacles[k]->obs)
      {
        Matrices.model = glm::mat4(1.0f);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(Obstacles[k]);
      }
    }
  }


  if(space)
  { 
    if(countt == 1)
    {
      if(sx <= 0)
        u += 10*sx;
      else
        u -= 10*sx;
   //   cout << "U = " << u << endl;
      countt--;  
    }
    Matrices.model = glm::mat4(1.0f);

    /* Render your scene */
  // triangle_translationX/20.0f

    glm::mat4 translatecannon = glm::translate (glm::vec3(sp_x + triangle_translationX/20.0f,sp_y + triangle_translationY/5.0f, 0.0f)); // glTranslatef
    glm::mat4 rotateconnon = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
    glm::mat4 cannonTransform = translatecannon*rotateconnon;
    Matrices.model *= cannonTransform; 
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(cannon);
    

    x_c = sp_x + triangle_translationX/20.0f;
    y_c = sp_y + triangle_translationY/5.0f;

    
    if(y_c <= -3.1)
    {
      t = 0;
      sp_x = x_c;
      sp_y = y_c + 0.1;
      if(xspeed > 0)
        angle = rectangle_rotation*(M_PI/180.0);
      else
        angle = M_PI - rectangle_rotation*(M_PI/180.0);
      u = e*u;
    }

    if(u < e*e*e*e*14)
    {
      triangle_translationX = 0;
      triangle_translationY = 0;
    }
    //camera_rotation_angle++; // Simulating camera rotation
    else
    {
      yspeed = u*sin(angle);
      xspeed = u*cos(angle);

      triangle_translationX = xspeed*t;
      if(!flag)
        triangle_translationY = yspeed*t - t*t;
      else
      {
        triangle_translationY = yspeed*t;
        flag--;
      }
      
      t=t+tspeed;
    }
  }


  //cout << "x aftr collision = " << x_c << " y after collison " << y_c << " y speed =" << yspeed << " x speed = " << u*cos(angle) << "\n";

  // if(triangle_translationX>70)
  //   triangle_translationX=1;

  // else if (triangle_translationX<2)
  // {
  //   triangle_translationX=1;
  // }



  // printf("%d\n",triangle_translationX );
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	drawCannon(0.0,0.0,0.0,0.05,360,false,false,0,0,0); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle1 (-2.0, -2.0, 0.0, 0.5, 8);
  createRectangle2 (-4.0, -4.0, 0.0, 8,5);
  createRectangle3(-4.0,-4.0,-0.0,8,85);
  createRectangle4(-4.0,3.9,0.0,8,5);
  createRectangle5(3.9,3.9,0.0,8,85);
  createRectangle6(-2.0,2.0,0.0,1,5);
  createRectangle7(-2.0,2.0,0.0,0.1,40);
 
	drawCircle1( 1.0, 1.0, 0.0, 0.1, 360,true,true, 255, 0,0);
  drawCircle2( 1.0, 2.0, 0.0, 0.1, 360,true ,true,0, 255, 0);
  drawCircle6( 0.0, -1.0, 0, 0.1, 360, true, true, 0,255,0);
  drawCircle7( 0.0, -2.0, 0, 0.2, 360, true, true, 0,255,0);
  drawCircle8( 0.0,  2.0, 0, 0.1, 360, true, false, 255,0,0);
  drawCircle4( 1.0, -2.0, 0, 0.1, 360, true, false, 0, 255,0);


  drawCircle4( -2.8, -2.0, 0, 0.4, 360, false, false, 0,0,0);
  drawCircle5( -2.8, -1.5, 0, 0.2, 360, false, false, 0,0,0);



	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    // cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    // cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    // cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    // cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void initvars()
{
  sp_x = -2.8,sp_y = -2.0;
  tspeed = 0.08, e = 0.7, g = -2.0, flag = 0, u = 15, t = 0;
  triangle_translationX = 1, triangle_translationY = 1;
  triangle_rotation = 45;
  
  rectangle_rot_status = false;
  rectangle_rotation = 45;
  space = 0;
  sx = 0.0;
  fla = 1;
  countt = 1;
  score = 0;
}

int main (int argc, char** argv)
{
	int width = 1920;
	int height = 1080;

  GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while(1)    
    {  
      initvars();
      GLfloat temp_x1 = -2.0, temp_y1 = -2.0, temp_x2, temp_y2;

      while (!glfwWindowShouldClose(window)) {

          
          // OpenGL Draw commands
          draw();

          temp_x2 = x_c;
          temp_y2 = y_c;

          if(temp_x1 == temp_x2 && temp_y1 == temp_y2 && space==1)
          { 
 //           cout << "Broken\n";
            break;
          }
          else
          {
            temp_x1 = temp_x2;
            temp_y1 = temp_y2;
          }

          // Swap Frame Buffer in double buffering
          glfwSwapBuffers(window);

          // Poll for Keyboard and mouse events
          glfwPollEvents();

          // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
          current_time = glfwGetTime(); // Time in seconds
          if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
              // do something every 0.5 seconds ..
              last_update_time = current_time;
          }
      }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
