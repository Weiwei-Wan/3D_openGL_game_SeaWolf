// Possible solution to labs 1 and 2
// If you have your glew, freeglut and GLM librarie setup, this should work if you just replace your main.cpp with this file
// The solution is not complete, e.g. you should put your shaders in text files and create a shader class using https://learnopengl.com/Getting-started/Shaders
// But it should help you better understand using multiple shader programmes, uniform variables and multible vertex buffers, keyboard/mouse control
// The main/most important content is in the init() and display() functions
// keyboard control using z and x keys

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

// These three includes will give us (almost?) everything we need from the GLM library for now
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

// Need to move these IDs here so that we can access them using the different functions
GLuint ShaderProgramID1;
GLuint ShaderProgramID2;
GLuint VBO1;
GLuint VBO2;

GLfloat RotateZ = 0.0f; // will use for rotation using keyboard control later on

// Define vertex and fragment shader programmes
// you can/(SHOULD?) place each shader in a .txt file and read in the file contents as a string rather than defining them as strings here

// Vertex Shader 1
// transforms each vertex according to the uniform variable M -> see display function for more on uniform variables
static const char* pVS1 = "                                                         \n\
#version 330                                                                        \n\
                                                                                    \n\
in vec3 vPosition;								    \n\
in vec4 vColor;									    \n\
									            \n\
uniform mat4 M;                                                                     \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
	gl_Position = M * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);         \n\
}";

// Fragment Shader 1
// Colours each fragment according to the uniform variable TriangleColor -> see display function for more on uniform variables
static const char* pFS1 = "                                                         \n\
#version 330                                                                        \n\
									            \n\
uniform vec4 TriangleColor;                                                         \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
	FragColor = TriangleColor;					            \n\
}";


// vertex shader 2
// does nothing other than pass the vertex positions and colours through 
static const char* pVS2 = "                                                        \n\
#version 330                                                                       \n\
                                                                                   \n\
in vec3 vPosition;								   \n\
in vec4 vColor;									   \n\
out vec4 color;									   \n\
                                                                                   \n\
void main()                                                                        \n\
{                                                                                  \n\
	gl_Position = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);            \n\
	color = vColor;							           \n\
}";

// Fragment Shader 2
// Colours fragments according to colours out put from vertex shader -> Notice how this is different to how we coloured fragments in pFS1
static const char* pFS2 = "                                                        \n\
#version 330                                                                       \n\
									           \n\
in vec4 color;                                                                     \n\
out vec4 FragColor;                                                                \n\
                                                                                   \n\
void main()                                                                        \n\
{                                                                                  \n\
	FragColor = color;         					           \n\
}";




// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderText, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* pVertexShaderText, const char* pFragmentShaderText)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, pVertexShaderText, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, pFragmentShaderText, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };


	// After compiling all shader objects and attaching them to the program, we can finally link it
	 glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
GLuint generateObjectBuffer(GLuint VBO, GLfloat vertices[], GLuint numVertices, GLfloat colors[] = NULL) {
	// Genderate 1 generic buffer object, called VBO
 	glGenBuffers(1, &VBO);
	// In OpenGL, we bind (make active) the handle to a target name and then execute commands on that target
	// Buffer will contain an array of vertices 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// After binding, we now fill our object with data, everything in "Vertices" goes to the GPU
	glBufferData(GL_ARRAY_BUFFER, numVertices*7*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	// if you have more data besides vertices (e.g., vertex colours or normals), use glBufferSubData to tell the buffer when the vertices array ends and when the colors start
	glBufferSubData (GL_ARRAY_BUFFER, 0, numVertices*3*sizeof(GLfloat), vertices);
	glBufferSubData (GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), numVertices*4*sizeof(GLfloat), colors);
	return VBO;
}

void linkCurrentBuffertoShader(GLuint shaderProgramID, GLuint numVertices){
	// find the location of the variables that we will be using in the shader program
	GLuint positionID = glGetAttribLocation(shaderProgramID, "vPosition");
	GLuint colorID = glGetAttribLocation(shaderProgramID, "vColor");
	// Have to enable this

	glEnableVertexAttribArray(positionID);
	// Tell it where to find the position data in the currently active buffer (at index positionID)
	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Similarly, for the color data.
	glEnableVertexAttribArray(colorID);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(numVertices*3*sizeof(GLfloat)));
}
#pragma endregion VBO_FUNCTIONS


void display(){

	glClear(GL_COLOR_BUFFER_BIT);

	// For every object we want to draw, we must
	// 1. Choose the shader programe we wish to use.
	glUseProgram(ShaderProgramID1);
	// 2. Choose the vertices/mesh/geometry we want to draw
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	// 3. link these choices
	linkCurrentBuffertoShader(ShaderProgramID1, 3);

	// 4. Set up any uniform variables in the shader, i.e. the matrix M and the vector TriangleColor in our case
	// Here we using GLM for matrices and vectors
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 R = glm::mat4(1.0f);
	glm::mat4 M = glm::mat4(1.0f);
	T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, +0.5f, 0.0f)); // T is matrix to move triangle up 0.5 in y-direction
	R = glm::rotate(glm::mat4(1.0f), RotateZ , glm::vec3(0.0f, 0.0f, 1.0f)); // R is matrix to rotate triangle about Z axis using keyboard control
	M = T * R;

	glm::vec4 TriangleColor = glm::vec4(1.0f);
	TriangleColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // make a red triangle
	
	// for each uniform variable we must a) create an ID using glGetUniformLocation; an
	// b) link our value for the uniform variable to this ID using glUniform*() (this function changes depending on what type the uniform varaible is (see next few lines)

	GLuint IDM = glGetUniformLocation(ShaderProgramID1, "M"); // Use these two lines to link Matrix M to uniform variable "M" in vertex shader
	glUniformMatrix4fv(IDM, 1, FALSE, glm::value_ptr(M));

	GLuint IDTriangleColor = glGetUniformLocation(ShaderProgramID1, "TriangleColor"); //  Similarly, link the vector TriangleColor to the uniform variable "TriangleColor" in the Shader program
	glUniform4fv(IDTriangleColor, 1, glm::value_ptr(TriangleColor));

	// 5. Draw the object
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Now we'll draw another triangle using the same VBO and shader programme 
	// But we'll changle the value of M so to move the triangle down and draw it below the first triangle
	// We'll change the color of the triangle to yellow
	// Thus, we only need to do step 4 (only the second part as we don't need new IDs) and step 5
	T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)); // change T to move triangle down 0.5 in y-direction
	M = T * R;
	glUniformMatrix4fv(IDM, 1, FALSE, glm::value_ptr(M)); // send new value of M to shader
	TriangleColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // make a yellow triangle
	glUniform4fv(IDTriangleColor, 1, glm::value_ptr(TriangleColor)); //send new value of TriangleColour to shader
	glDrawArrays(GL_TRIANGLES, 0, 3); // draw new trianlge



	// Next we'll use shader program 2 and VBO2 to draw the two triangles on the left of the window
	// 1. Choose the shader programme we wish to use
	glUseProgram(ShaderProgramID2);
	// 2. Choose the vertices/mesh/geometry we want to draw
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	// 3. link these choices
	linkCurrentBuffertoShader(ShaderProgramID2, 6);
	// 4. Set up any uniform variables in the shader
	// We don't have any
	// 5. Draw the object
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glutSwapBuffers();
}


void init()
{
	// these three vertices will be used to draw a trianlge in top right of window
	// and a triangle in the bottom right of the window
	// these vertices will be used with shader programme 1
	GLfloat vertices[] = { 0.0f, 0.5f, 0.0f,
			0.0f, -0.5f, 0.0f,
			1.0f, 0.0f, 0.0f };

	// first 3 vertices will be triangle in bottom left of window
	// second 3 vertices will be triangle in top left of window
	// these vertices will be used with shader programme 2
	GLfloat vertices2[] = { -1.0f, -1.0f, 0.0f,
			0.0f, -1.0f, 0.0f,
			-0.5f, 0.0f, 0.0f,
			-0.5f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f };

	// Create a color array that identfies the colors of each vertex (format R, G, B, A)
	// will use these colours with Shader Programme 2
	// the colours for shader programme 1 will be set up using uniform variables in the display function 
	GLfloat colors[] = {0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f
	};

	// Put the vertices and colors into a vertex buffer object
	VBO1 = generateObjectBuffer(VBO1, vertices, 3);
	VBO2 = generateObjectBuffer(VBO2, vertices2, 6, colors);

	// Set up the shaders
	// it will help in future if you set up a shader class. This can be done by following https://learnopengl.com/Getting-started/Shaders
	// also, you should switch pVS1, pVS2, pFS1, pFS2 to .txt files and read them in as strings
	ShaderProgramID1 = CompileShaders(pVS1, pFS1);
	ShaderProgramID2 = CompileShaders(pVS2, pFS2);
}

// function to allow keyboard control
// it's called a callback function and must be registerd in main() using glutKeyboardFunc();
// the functions must be of a specific format - see freeglut documentation
// similar functions exist for mouse control etc
void keyPress(unsigned char key, int x, int y)
{
	switch (key) {
	case 'z':
		RotateZ -= 1.0f;
		break;
	case 'x':
		RotateZ += 1.0f;
		break;
	}

	// we must call these to redraw the scene after we make any changes 
	glutPostRedisplay();
}



int main(int argc, char** argv){

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Hello Triangle");
	
	// Register Call Back funtions
	glutDisplayFunc(display);
	glutKeyboardFunc(keyPress); // allows for keyboard control. See keyPress function above

	 // A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}