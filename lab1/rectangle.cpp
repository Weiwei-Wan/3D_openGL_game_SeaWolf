#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

// Vertex Shader (for convenience, it is defined in the main here, but we will be using text files for shaders in future)
// Note: Input to this shader is the vertex positions that we specified for the triangle.
// Note: gl_Position is a special built-in variable that is supposed to contain the vertex position (in X, Y, Z, W)
// Since our triangle vertices were specified as vec3, we just set W to 1.0.

static const char* pVS = "                  \n\
                attribute vec3 vPosition;  \n\
                attribute vec3 vColor;  \n\
                varying vec4 pass_Color;  \n\
                void main(void) {  \n\
                    gl_Position = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);   \n\
                    pass_Color = vec4(vColor, 1.0);   \n\
                }";

// Fragment Shader
// Note: no input in this shader, it just outputs the colour of all fragments, in this case set to red (format: R, G, B, A).
static const char* pFS = "                                              \n\
                varying vec4 pass_Color;  \n\
                void main()                                                               \n\
                {                                                                          \n\
                    gl_FragColor = pass_Color;  \n\
                }";

GLfloat vertices[]=
{
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.5f,  0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f,
};

GLuint indices[] = {
    0, 1, 2,
    2, 3, 0,
};

GLfloat colors[] = {0.3f, 0.5f, 0.1f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
};

GLuint numVertices = 6;
GLuint VBO1, VBO2;
GLuint ShaderProgramID;

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

GLuint CompileShaders()
{
    //Start the process of setting up our shaders by creating a program ID
    //Note: we will link all the shaders together into this ID
    GLuint shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }
    // Create two shader objects, one for the vertex, and one for the fragment shader
    AddShader(shaderProgramID, pVS, GL_VERTEX_SHADER);
    AddShader(shaderProgramID, pFS, GL_FRAGMENT_SHADER);

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

GLuint generateObjectBuffer(GLfloat vertices[], GLfloat colors[]) {
    // Genderate 1 generic buffer object, called VBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    // In OpenGL, we bind (make active) the handle to a target name and then execute commands on that target
    // Buffer will contain an array of vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // After binding, we now fill our object with data, everything in "Vertices" goes to the GPU
    glBufferData(GL_ARRAY_BUFFER, numVertices*7*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    // if you have more data besides vertices (e.g., vertex colours or normals), use glBufferSubData to tell the buffer when the vertices array ends and when the colors start
    glBufferSubData (GL_ARRAY_BUFFER, 0, numVertices*3*sizeof(GLfloat), vertices);
    glBufferSubData (GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), numVertices*4*sizeof(GLfloat), colors);
    
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numVertices*sizeof(GLfloat), indices, GL_STATIC_DRAW);
    return VBO;
}

void linkCurrentBuffertoShader(GLuint shaderProgramID){
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
    glClearColor(0.8f,0.8f,0.8f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // For every object we want to draw, we must
    // 1. Choose the shader programe we wish to use.
    glUseProgram(ShaderProgramID);
    // 2. Choose the vertices/mesh/geometry we want to draw
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    // 3. link these choices
    linkCurrentBuffertoShader(ShaderProgramID);
    // NB: Make the call to draw the geometry in the currently activated vertex buffer. This is where the GPU starts to work!
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glutSwapBuffers();
}
void init()
{
    // Set up the shaders
    ShaderProgramID = CompileShaders();
    // Put the vertices and colors into a vertex buffer object
    VBO1 = generateObjectBuffer(vertices, colors);
}

int main(int argc, char** argv){

    // Set up the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Hello Triangle");
    // Tell glut where the display function is
    glutDisplayFunc(display);
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

