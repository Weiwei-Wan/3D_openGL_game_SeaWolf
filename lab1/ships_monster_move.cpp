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
#define MESH_NAME "watercraftPack_023.obj"
#define MESH_NAME2 "eyeball.dae"
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

ModelData mesh_data;
ModelData mesh_data2;

unsigned int mesh_vao = 0;
int width = 1000;
int height = 800;

// Root of the Hierarchy
glm::mat4 persp_proj = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
// Camera matrix
glm::mat4 view1       = glm::lookAt(
                            glm::vec3(15,15,0), // Camera is at (4,3,-3), in World Space
                            glm::vec3(0,0,0), // and looks at the origin
                            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                        );
glm::mat4 view2       = glm::lookAt(
                            glm::vec3(15,15,-5), // Camera is at (4,3,-3), in World Space
                            glm::vec3(0,0,0), // and looks at the origin
                            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                        );
glm::mat4 view = view1;
// Model matrix : an identity matrix (model will be at the origin)
glm::mat4 model      = glm::mat4(1.0f);

GLuint loc1, loc2, loc3;
GLfloat rotate_y = 0.0f;


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
    //    unsigned int vt_vbo = 0;
    //    glGenBuffers (1, &vt_vbo);
    //    glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
    //    glBufferData (GL_ARRAY_BUFFER, monkey_head_data.mTextureCoords * sizeof (vec2), &monkey_head_data.mTextureCoords[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(loc1);
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(loc2);
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    //    This is for texture coordinates which you don't currently need, so I have commented it out
    //    glEnableVertexAttribArray (loc3);
    //    glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
    //    glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
}
#pragma endregion VBO_FUNCTIONS


void display() {

    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // draw two ships
    glUseProgram(shaderProgramID);
    generateObjectBufferMesh(mesh_data);
    
    static float Scale = 0.0f;
    static float Delta = 0.005f;
    Scale += Delta;
    if ((Scale >= 15.0f) || (Scale <= -15.0f)) {
        Delta *= -1.0f;
    }
    glm::mat4 Translation(1.0f, 0.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f, Scale,
                          0.0f, 0.0f, 0.0f, 1.0f);

    //Declare your uniform variables that will be used in your shader
    int matrix_location = glGetUniformLocation(shaderProgramID, "model");
    int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
    int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
    int gTranslationLocation = glGetUniformLocation(shaderProgramID, "Translation");


    // update uniforms & draw
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &persp_proj[0][0]);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(gTranslationLocation, 1, GL_TRUE, &Translation[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
    
    static float Scale2 = 0.0f;
    static float Delta2 = 0.01f;
    Scale2 += Delta2;
    if ((Scale2 >= 15.0f) || (Scale2 <= -15.0f)) {
        Delta2 *= -1.0f;
    }
    glm::mat4 Translation2(1.0f, 0.0f, 0.0f, 2.5f,
                           0.0f, 1.0f, 0.0f, -2.5f,
                           0.0f, 0.0f, 1.0f, Scale2,
                           0.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 model2      = glm::mat4(2.0f);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &persp_proj[0][0]);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, &model2[0][0]);
    glUniformMatrix4fv(gTranslationLocation, 1, GL_TRUE, &Translation2[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
    
    //draw the mosters
    static float Scale3 = 0.0f;
    static float Delta3 = -0.008f;
    Scale3 += Delta3;
    if ((Scale3 >= 15.0f) || (Scale3 <= -15.0f)) {
        Delta3 *= -1.0f;
    }
    glUseProgram(shaderProgramID);
    generateObjectBufferMesh(mesh_data2);
    glm::mat4 Translation3(1.0f, 0.0f, 0.0f, 5.0f,
                           0.0f, 1.0f, 0.0f, -5.0f,
                           0.0f, 0.0f, 1.0f, Scale3,
                           0.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 model3      = glm::mat4(3.0f);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &persp_proj[0][0]);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, &model3[0][0]);
    glUniformMatrix4fv(gTranslationLocation, 1, GL_TRUE, &Translation3[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, mesh_data2.mPointCount);
    
    glutPostRedisplay();
    glutSwapBuffers();
}


void init()
{
    // Set up the shaders
    shaderProgramID = CompileShaders();
    // load mesh into a vertex buffer array
    mesh_data = load_mesh(MESH_NAME);
    mesh_data2 = load_mesh(MESH_NAME2);
    //mesh_data3 = load_mesh(MESH_NAME3);
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
    if (key == 'x') {
        printf("GLUT_KEY_X");
        if (view == view1) {
            view = view2;
        }
        else {
            view = view1;
        }
    }
}

int main(int argc, char** argv) {

    // Set up the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Hello Triangle");

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
