/*
const char* pVSFileName = "vsShader.vs";
const char* pFSFileName = "fsShader.fs";
 
string vs, fs;

ifstream vsFile(pVSFileName);
stringstream vsBuff;
vsBuff << vsFile.rdbuf();
vs = vsBuff.str();

AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

ifstream fsFile(pFSFileName);
stringstream fsBuff;
fsBuff << fsFile.rdbuf();
fs = fsBuff.str();

AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

 const char* pVSFileName = "vsShader.vs";
 const char* pFSFileName = "fsShader.fs";
*/

#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

using namespace std;

#include "maths_funcs.h"

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_SHIP "watercraftPack_023.obj"
#define MESH_FISH "Shark.dae"
#define MESH_NAME3 "Heart.dae"
#define MESH_BULLET "bullet.dae"

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

#pragma region SimpleTypes
typedef struct
{
    size_t mPointCount = 0;
    std::vector<vec3> mVertices;
    std::vector<vec3> mNormals;
    std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;
GLuint shaderProgramID;
unsigned int vp_vbo = 0;
unsigned int vn_vbo = 0;
unsigned int vt_vbo = 0;

ModelData mesh_ship;
ModelData mesh_fish;
ModelData mesh_data3;
ModelData mesh_bullet;

unsigned int mesh_vao = 0;
int width = 1000;
int height = 800;

// Root of the Hierarchy
glm::mat4 persp_proj = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
// Camera pos
GLfloat camera_pos_z = 0.0f;
GLfloat camera_pos_y = 30.0f;
// Model matrix : an identity matrix (model will be at the origin)
glm::mat4 model      = glm::mat4(1.0f);

static float pos_ship1 = 0.0f;
// speed
static float Delta_ship1 = 0.005f;
static float pos_ship2 = 0.0f;
static float Delta_ship2 = 0.01f;
static float pos_fish = 0.0f;
static float Delta_fish = -0.008f;
static float pos_player = 0.0f;
static float Delta_player = 1.0f;
static float pos_bullet = 10.0f;
static float Delta_bullet = 0.01f;
static float Bullet_pos_z = 0.0f;
bool has_bullet = false;
int score = 0;
static float pos0 = 40.0f;

GLuint loc1, loc2, loc3;

#define BMP_Header_Length 54

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
    ModelData modelData;

    /* Use assimp to read the model file, forcing it to be read as    */
    /* triangles. The second flag (aiProcess_PreTransformVertices) is */
    /* relevant if there are multiple meshes in the model file that   */
    /* are offset from the origin. This is pre-transform them so      */
    /* they're in the right position.                                 */
    const aiScene* scene = aiImportFile(
        file_name,
        aiProcess_Triangulate | aiProcess_PreTransformVertices
    );

    if (!scene) {
        fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
        return modelData;
    }

    printf("  %i materials\n", scene->mNumMaterials);
    printf("  %i meshes\n", scene->mNumMeshes);
    printf("  %i textures\n", scene->mNumTextures);

    for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
        const aiMesh* mesh = scene->mMeshes[m_i];
        printf("    %i vertices in mesh\n", mesh->mNumVertices);
        modelData.mPointCount += mesh->mNumVertices;
        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions()) {
                const aiVector3D* vp = &(mesh->mVertices[v_i]);
                modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
            }
            if (mesh->HasNormals()) {
                const aiVector3D* vn = &(mesh->mNormals[v_i]);
                modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
            }
            if (mesh->HasTextureCoords(0)) {
                const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
                modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
            }
            if (mesh->HasTangentsAndBitangents()) {
                /* You can extract tangents and bitangents here              */
                /* Note that you might need to make Assimp generate this     */
                /* data for you. Take a look at the flags that aiImportFile  */
                /* can take.                                                 */
            }
        }
    }

    aiReleaseImport(scene);
    return modelData;
}
#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    // create a shader object
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        std::cerr << "Error creating shader..." << std::endl;
        std::cerr << "Press enter/return to exit..." << std::endl;
        std::cin.get();
        exit(1);
    }

    ifstream vsFile(pShaderText);
    stringstream vsBuff;
    vsBuff << vsFile.rdbuf();
    string vs = vsBuff.str();
    const GLchar* p[1];
    p[0] = vs.c_str();
    // Bind the source code to the shader, this happens before compilation
    glShaderSource(ShaderObj, 1, p, NULL);
    // compile the shader and check for errors
    glCompileShader(ShaderObj);
    GLint success;
    // check for shader related errors using glGetShaderiv
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024] = { '\0' };
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        std::cerr << "Error compiling "
            << (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << " shader program: " << InfoLog << std::endl;
        std::cerr << "Press enter/return to exit..." << std::endl;
        std::cin.get();
        exit(1);
    }
    // Attach the compiled shader object to the program object
    glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
    //Start the process of setting up our shaders by creating a program ID
    //Note: we will link all the shaders together into this ID
    shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        std::cerr << "Error creating shader program..." << std::endl;
        std::cerr << "Press enter/return to exit..." << std::endl;
        std::cin.get();
        exit(1);
    }

    // Create two shader objects, one for the vertex, and one for the fragment shader
    AddShader(shaderProgramID, "vsShader.vs", GL_VERTEX_SHADER);
    AddShader(shaderProgramID, "fsShader.fs", GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { '\0' };
    // After compiling all shader objects and attaching them to the program, we can finally link it
    glLinkProgram(shaderProgramID);
    // check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
        std::cerr << "Press enter/return to exit..." << std::endl;
        std::cin.get();
        exit(1);
    }

    // program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
    glValidateProgram(shaderProgramID);
    // check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
        std::cerr << "Press enter/return to exit..." << std::endl;
        std::cin.get();
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
void generateObjectBufferMesh(ModelData mesh_data) {
    /*----------------------------------------------------------------------------
    LOAD MESH HERE AND COPY INTO BUFFERS
    ----------------------------------------------------------------------------*/

    //Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
    //Might be an idea to do a check for that before generating and binding the buffer.

    vp_vbo = 0;
    loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
    loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
    loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

    glGenBuffers(1, &vp_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);
    vn_vbo = 0;
    glGenBuffers(1, &vn_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

    //    This is for texture coordinates which you don't currently need, so I have commented it out
    vt_vbo = 0;
    glGenBuffers (1, &vt_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
    glBufferData (GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec2), &mesh_data.mTextureCoords[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(loc1);
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(loc2);
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray (loc3);
    glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
    glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
}
#pragma endregion VBO_FUNCTIONS

void check_collision() {
    // meet fish
    if (pos_bullet < 11.0f and pos_bullet > 10.0f) {
        if (Bullet_pos_z > pos_fish-5.0f and Bullet_pos_z < pos_fish+5.0f) {
            has_bullet = false;
        }
    }
    // meet ship2
    if (pos_bullet < 6.0f and pos_bullet > 5.0f) {
        if (Bullet_pos_z > pos_ship2-3.0f and Bullet_pos_z < pos_ship2+3.0f) {
            has_bullet = false;
            pos_ship2 = -29.0f;
            score += 1;
        }
    }
    // meet ship1
    if (pos_bullet < 1.0f and pos_bullet > 0.0f) {
        if (Bullet_pos_z > pos_ship1-3.0f and Bullet_pos_z < pos_ship1+3.0f) {
            has_bullet = false;
            pos_ship1 = 29.0f;
            score += 1;
        }
    }
}

void display() {
    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (has_bullet) {check_collision();}
    
    // draw two ships
    glUseProgram(shaderProgramID);
    generateObjectBufferMesh(mesh_ship);
    
    glm::mat4 view = glm::lookAt(glm::vec3(20,camera_pos_y,camera_pos_z), // Camera is at (x,y,z), in World Space
                                glm::vec3(0,0,0), // and looks at the origin
                                glm::vec3(0,1,0));  // Head is up (set to 0,-1,0 to look upside-down)
    
    //Declare your uniform variables that will be used in your shader
    int model_color = glGetUniformLocation(shaderProgramID, "color");
    int matrix_location = glGetUniformLocation(shaderProgramID, "model");
    int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
    int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
    int gTransformationLocation = glGetUniformLocation(shaderProgramID, "Translation");
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &persp_proj[0][0]);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, &model[0][0]);
    
    pos_ship1 += Delta_ship1;
    if ((pos_ship1 >= 30.0f) || (pos_ship1 <= -30.0f)) {
        Delta_ship1 *= -1.0f;
    }
    glm::mat4 Transformation(1.0f, 0.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f, pos_ship1,
                          0.0f, 0.0f, 0.0f, 1.0f);
    // update uniforms & draw
    glUniformMatrix4fv(gTransformationLocation, 1, GL_TRUE, &Transformation[0][0]);
    glUniform3f(model_color, 0.3f, 0.5f, 0.1f);
    glDrawArrays(GL_TRIANGLES, 0, mesh_ship.mPointCount);
    
    pos_ship2 += Delta_ship2;
    if ((pos_ship2 >= 30.0f) || (pos_ship2 <= -30.0f)) {
        Delta_ship2 *= -1.0f;
    }
    glm::mat4 Transformation2(1.0f, 0.0f, 0.0f, 5.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, pos_ship2,
                           0.0f, 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(gTransformationLocation, 1, GL_TRUE, &Transformation2[0][0]);
    glUniform3f(model_color, 0.8f, 0.06f, 0.46f);
    glDrawArrays(GL_TRIANGLES, 0, mesh_ship.mPointCount);
    
    // player
    glm::mat4 Transformation5(1.0f, 0.0f, 0.0f, 15.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, pos_player,
                           0.0f, 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(gTransformationLocation, 1, GL_TRUE, &Transformation5[0][0]);
    glUniform3f(model_color, 0.28f, 0.24f, 0.55f);
    glDrawArrays(GL_TRIANGLES, 0, mesh_ship.mPointCount);
    
    //draw the fish
    pos_fish += Delta_fish;
    if ((pos_fish >= 25.0f) || (pos_fish <= -25.0f)) {
        Delta_fish *= -1.0f;
    }
    generateObjectBufferMesh(mesh_fish);
    glm::mat4 Transformation3(0.5f, 0.0f, 0.0f, 10.0f,
                           0.0f, 0.5f, 0.0f, 0.0f,
                           0.0f, 0.0f, 0.5f, pos_fish,
                           0.0f, 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(gTransformationLocation, 1, GL_TRUE, &Transformation3[0][0]);
    glUniform3f(model_color, 1.0f, 0.5f, 0.28f);
    glDrawArrays(GL_TRIANGLES, 0, mesh_fish.mPointCount);
    
    // draw the heart
    pos0 = 40.0f;
    for (int i=0; i<score; i++) {
        generateObjectBufferMesh(mesh_data3);
        glm::mat4 Transformation4(2.0f, -1.0f, 0.0f, -40.0f,
                                0.0f, 3.0f, 0.0f, 0.0f,
                                0.0f, -1.0f, 2.0f, pos0,
                                0.0f, 0.0f, 0.0f, 2.0f);
        glUniformMatrix4fv(gTransformationLocation, 1, GL_TRUE, &Transformation4[0][0]);
        glUniform3f(model_color, 1.0f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, mesh_data3.mPointCount);
        pos0 -= 5.0f;
    }
    
    // bullet
    if (has_bullet) {
        pos_bullet -= Delta_bullet;
        if (pos_bullet < -3.0f) { has_bullet = false;}
        generateObjectBufferMesh(mesh_bullet);
        glm::mat4 Transformation6(0.5f, 0.0f, 0.0f, pos_bullet,
                               0.0f, 0.5f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.5f, Bullet_pos_z,
                               0.0f, 0.0f, 0.0f, 1.0f);
        glUniformMatrix4fv(gTransformationLocation, 1, GL_TRUE, &Transformation6[0][0]);
        glUniform3f(model_color, 0.0f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, mesh_bullet.mPointCount);
    }
    
    glutPostRedisplay();
    glutSwapBuffers();
}

void init()
{
    // Set up the shaders
    shaderProgramID = CompileShaders();
    // load mesh into a vertex buffer array
    mesh_ship = load_mesh(MESH_SHIP);
    mesh_fish = load_mesh(MESH_FISH);
    mesh_data3 = load_mesh(MESH_NAME3);
    mesh_bullet = load_mesh(MESH_BULLET);
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
    // change the camera position
    if (key == 'l') {
        camera_pos_z += 5.0f;
    }
    else if (key == 'j') {
        camera_pos_z -= 5.0f;
    }
    else if (key == 'i') {
        camera_pos_y += 5.0f;
    }
    else if (key == 'k') {
        camera_pos_y -= 5.0f;
    }
    // change player ship position
    else if (key == 'a') {
        printf("GLUT_KEY_left");
        pos_player += Delta_player;
        if ((pos_player >= 25.0f) || (pos_player <= -25.0f)) {
            Delta_player *= -1.0f;
        }
    }
    else if (key == 'd') {
        printf("GLUT_KEY_right");
        pos_player -= Delta_player;
        if ((pos_player >= 25.0f) || (pos_player <= -25.0f)) {
            Delta_player *= -1.0f;
        }
    }
    // shoot
    else if (key == 't') {
        has_bullet = true;
        Bullet_pos_z = pos_player;
        pos_bullet = 15.0f;
    }
}

int main(int argc, char** argv) {

    // Set up the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Sea Wolf");

    //texure
    glEnable(GL_DEPTH_TEST);
    
    // Tell glut where the display function is
    glutDisplayFunc(display);
    glutKeyboardFunc(keypress);

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
