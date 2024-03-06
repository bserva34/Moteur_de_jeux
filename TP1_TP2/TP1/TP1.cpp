// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <thread>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/controls.hpp>

#include "FastNoiseLit.h"

#include <common/text2D.hpp>
#include "TP1/Texture.cpp"

using namespace glm;
using namespace std;

void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static FastNoiseLite noise;

// camera

glm::vec3 camera_rotate_position = glm::vec3(0.0f, 0.0f, 20.0f); 
glm::vec3 camera_free_position = glm::vec3(0.0f, 0.0f, 20.0f);
glm::vec3 camera_position   = camera_rotate_position;
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up    = glm::vec3(0.0f, 1.0f,  0.0f);
glm::vec3 cameraPosOrbit = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraOrbitTarget = glm::vec3(0., 0., -1.);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
float speed_ = 1.f;

//rotation
float angle = 0.f;
float vertical_angle=0.f;
float zoom = 1.;
float camera_inclination_angle = 45.0f; 






bool cam=true;
int resolution=16;
    
int nx_square=8;
int ny_square=8;

std::vector<unsigned short> indices; //Triangles concaténés dans une liste
std::vector<std::vector<unsigned short> > triangles;
std::vector<glm::vec3> indexed_vertices;
std::vector<glm::vec2> indexed_texcoords;

GLuint vertexbuffer;
GLuint elementbuffer;
GLuint texcoordbuffer;
/*******************************************************************************/

void creation_plan(std::vector<unsigned short> &indices, std::vector<std::vector<unsigned short> > &triangles, std::vector<glm::vec3> &indexed_vertices, std::vector<glm::vec2> &indexed_texcoords) {
    indices.clear();
    indexed_vertices.clear();
    indexed_texcoords.clear();

    
    for (int i = 0; i <= resolution; i++) {
        for (int j = 0; j <= resolution; j++) {
            float x = static_cast<float>(i) / resolution - 0.5f;
            float y = static_cast<float>(j) / resolution - 0.5f;
            glm::vec3 val = glm::vec3((float)x * nx_square, 0, (float)y * ny_square);
            indexed_vertices.push_back(val);

            // Calcul des coordonnées de texture normalisées [0, 1]
            glm::vec2 texCoord = glm::vec2(static_cast<float>(i) / resolution, static_cast<float>(j) / resolution);
            indexed_texcoords.push_back(texCoord);
        }
    }

    /*
    for (int i = 0; i <= resolution; i++) {
        for (int j = 0; j <= resolution; j++) {
            float x = static_cast<float>(i) / resolution - 0.5f;
            float y = static_cast<float>(j) / resolution - 0.5f;
            glm::vec3 val = glm::vec3((float)x * nx_square, (float)y * ny_square,0);
            indexed_vertices.push_back(val);

            // Calcul des coordonnées de texture normalisées [0, 1]
            glm::vec2 texCoord = glm::vec2(static_cast<float>(i) / resolution, static_cast<float>(j) / resolution);
            indexed_texcoords.push_back(texCoord);
        }
    }*/

    for (int i = 0; i < resolution; i++) {
        for (int j = 0; j < resolution; j++) {
            // Triangle 1
            indices.push_back(i * (resolution + 1) + j);
            indices.push_back((i + 1) * (resolution + 1) + j);
            indices.push_back(i * (resolution + 1) + j + 1);

            // Triangle 2
            indices.push_back(i * (resolution + 1) + j + 1);
            indices.push_back((i + 1) * (resolution + 1) + j);
            indices.push_back((i + 1) * (resolution + 1) + j + 1);
        }
    }
}



glm::mat4 view;
glm::mat4 model;
glm::mat4 projection;


int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "TP1 - GLFW", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    //  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    //glEnable(GL_CULL_FACE);
    //glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    creation_plan(indices,triangles,indexed_vertices,indexed_texcoords);

     // Load it into a VBO


    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    glGenBuffers(1, &texcoordbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_texcoords.size() * sizeof(glm::vec2), &indexed_texcoords[0], GL_STATIC_DRAW);


    
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    //CheckProgramLinking(programID);
    /*****************TODO***********************/
    
    
    GLint carte = loadTexture2DFromFilePath("./texture/Heightmap_Mountain.png");
    //GLint carte = loadTexture2DFromFilePath("./texture/heightmap.png");
    //GLint snow = loadTexture2DFromFilePath("./texture/snowrocks.png");
    GLint snow = loadTexture2DFromFilePath("./texture/SNOW.png");
    GLint rock = loadTexture2DFromFilePath("./texture/rock.png");
    GLint grass = loadTexture2DFromFilePath("./texture/grass.png");
    

    /****************************************/
    

    //Chargement du fichier de maillage
    

    //std::string filename("chair.off");
    //loadOFF(filename, indexed_vertices, indices, triangles );

    if (carte != -1) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, carte);        
            glUniform1i(glGetUniformLocation(programID,"carte"), 0);
            
            }
    if (snow != -1) {
    glActiveTexture(GL_TEXTURE0+1);
    glBindTexture(GL_TEXTURE_2D, snow);        
    glUniform1i(glGetUniformLocation(programID,"snow"), 1);

    }
    if (grass != -1) {
    glActiveTexture(GL_TEXTURE0+2);
    glBindTexture(GL_TEXTURE_2D, grass);        
    glUniform1i(glGetUniformLocation(programID,"grass"), 2);

    }
    if (rock != -1) {
    glActiveTexture(GL_TEXTURE0+3);
    glBindTexture(GL_TEXTURE_2D, rock);        
    glUniform1i(glGetUniformLocation(programID,"rock"), 3);

    }
        


    	

   



    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{
        


        

        // Measure speed
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glUseProgram(programID);
        glm::vec3 camera_position_used;
        if (cam) {
            camera_position_used = camera_rotate_position;
        } else {
            camera_position_used = camera_free_position;
        }

        if (cam) {
            angle += speed_ * deltaTime * 70;  
            view = glm::lookAt(camera_position_used, camera_position_used + camera_target, camera_up);
            model = mat4(1.0f);
            model = glm::rotate(model, glm::radians(45.f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));  
            model = glm::scale(model, glm::vec3(zoom, zoom, zoom));         
            projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        }
        else{
            

            view = glm::lookAt(camera_position_used, camera_position_used + camera_target, camera_up);
            model = mat4(1.0f);
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); // rotation example
            model = glm::scale(model, glm::vec3(zoom, zoom, zoom));          
            projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        }

       

            
        
    	    glm::mat4 MVP = projection * view * model;

            glUniformMatrix4fv(glGetUniformLocation(programID, "model"), 1, GL_FALSE, &model[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(programID, "view"), 1, GL_FALSE, &view[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(programID, "projection"), 1, GL_FALSE, &projection[0][0]);
        
        glUniform1f(glGetUniformLocation(programID, "resolution"),resolution);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                    0,                  // attribute
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
                    );


        GLuint texcoordAttrib = glGetAttribLocation(programID, "texCoord");
        glEnableVertexAttribArray(texcoordAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, texcoordbuffer);
        glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);



        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,      // mode
                    indices.size(),    // count
                    GL_UNSIGNED_SHORT,   // type
                    (void*)0           // element array buffer offset
                    );

        glDisableVertexAttribArray(0);


        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );
    
    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        cout << "touche c utilisé" << endl;
        cam = !cam;
        if(cam){
            angle = 0.0f;
            model = glm::mat4(1.0f);
            speed_=1.f;
        }else{
        angle = 0.f;
        camera_free_position = glm::vec3(0.0f, 0.0f, 20.0f);
        view = glm::lookAt(camera_free_position, camera_free_position + camera_target, camera_up);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
        resolution++;
        creation_plan(indices,triangles,indexed_vertices,indexed_texcoords);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, texcoordbuffer);
        glBufferData(GL_ARRAY_BUFFER, indexed_texcoords.size() * sizeof(glm::vec2), &indexed_texcoords[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);
    }

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS){
        if(resolution>1){
            resolution--;
            creation_plan(indices,triangles,indexed_vertices,indexed_texcoords);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, texcoordbuffer);
            glBufferData(GL_ARRAY_BUFFER, indexed_texcoords.size() * sizeof(glm::vec2), &indexed_texcoords[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);
        }
        
    }

    if (cam) {
        
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            speed_+=0.1;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            speed_-=0.1;
        }
        

    } else {

        float cameraSpeed = 2.5 * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera_free_position += glm::vec3(0.0, 0.0, 0.1);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera_free_position -= glm::vec3(0.0, 0.0, 0.1);
        }

        glm::vec3 direction(
            cos(vertical_angle) * sin(angle),
            sin(vertical_angle),
            cos(vertical_angle) * cos(angle)
        );

        
        glm::vec3 right = glm::vec3(
            sin(angle - 3.14f / 2.0f),
            0,
            cos(angle - 3.14f / 2.0f)
        );
        glm::vec3 up = glm::cross(right, direction);

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            camera_free_position += up * deltaTime * 50.f;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            camera_free_position -= up * deltaTime * 50.f;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            camera_free_position -= right * deltaTime * 50.f;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            camera_free_position += right * deltaTime * 50.f;
        }

        view = glm::lookAt(camera_free_position, camera_free_position + camera_target, camera_up);
    }

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
