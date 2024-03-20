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

bool sens=true;






bool cam=true;
    


std::vector<unsigned short> indices; //Triangles concaténés dans une liste
std::vector<std::vector<unsigned short> > triangles;
std::vector<glm::vec3> indexed_vertices;
std::vector<glm::vec2> indexed_texcoords;

GLuint vertexbuffer;
GLuint elementbuffer;
GLuint texcoordbuffer;
/*******************************************************************************/

struct Noeud{
    std::vector<vec3> id;
    std::vector<unsigned short> tri;
    Noeud  * pere;
    std::vector<Noeud*> enfant; 
    mat4 model;
    GLuint buffer_un;
    GLuint buffer_deux;
    vec3 color;
};

void setup_buffer(Noeud &courant,GLuint &vb,GLuint &eb){
    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, courant.id.size() * sizeof(glm::vec3), &courant.id[0], GL_STATIC_DRAW);

    glGenBuffers(1, &eb);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, courant.tri.size() * sizeof(unsigned short), &courant.tri[0] , GL_STATIC_DRAW);
}

void send_buffer(Noeud &courant,GLuint &vb, GLuint &eb){
	glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glVertexAttribPointer(
                0,                  // attribute
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
                );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb);

    // Draw the triangles !
    glDrawElements(
                GL_TRIANGLES,      // mode
                courant.tri.size(),    // count
                GL_UNSIGNED_SHORT,   // type
                (void*)0           // element array buffer offset
                );

    glDisableVertexAttribArray(0);
}

void send_matrice(mat4 model,mat4 view,mat4 projection,GLuint programID){
    glUniformMatrix4fv(glGetUniformLocation(programID, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(programID, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(programID, "projection"), 1, GL_FALSE, &projection[0][0]);
}



void creation_plan(std::vector<unsigned short> &indices, std::vector<glm::vec3> &indexed_vertices,vec3 pdd) {
    indices.clear();
    indexed_vertices.clear();

    indexed_vertices.push_back(vec3(pdd[0]-1,pdd[1]-1,pdd[2]));
    indexed_vertices.push_back(vec3(pdd[0]+1,pdd[1]+1,pdd[2]));
    indexed_vertices.push_back(vec3(pdd[0]-1,pdd[1]+1,pdd[2]));
    indexed_vertices.push_back(vec3(pdd[0]+1,pdd[1]-1,pdd[2]));
    

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(3);


}
void set_model(mat4 m,Noeud &racine){
    racine.model=m;
    //cout<<racine.enfant.size()<<endl;
    for(int i=0;i<racine.enfant.size();i++){
        set_model(m,*racine.enfant[i]);
    }
}

void send(GLuint programID,mat4 view,mat4 projection,Noeud &racine){
    setup_buffer(racine,racine.buffer_un,racine.buffer_deux);

    send_matrice(racine.model,view,projection,programID);

    glUniform3f(glGetUniformLocation(programID,"cb"),racine.color[0],racine.color[1],racine.color[2]);

    send_buffer(racine,racine.buffer_un,racine.buffer_deux);
    for(int i=0;i<racine.enfant.size();i++){
        send(programID,view,projection,*racine.enfant[i]);
    }
}



glm::mat4 view;
glm::mat4 model;
glm::mat4 projection;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

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
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );


    /*
    Noeud test;
    creation_plan(test.tri,test.id,vec3(0.,0.,0.));

    Noeud test2;
    test2.pere = &test;
    creation_plan(test2.tri,test2.id,vec3(-2.,-3.,0.));

    Noeud test3;
    test3.pere = &test;
    creation_plan(test3.tri,test3.id,vec3(2.,-3.,0.));


    test.enfant.push_back(&test2);
    test.enfant.push_back(&test3);*/

    Noeud soleil;
    std::vector< std::vector<unsigned short > > triangles ;
    loadOFF("../data/sphere.off",soleil.id,soleil.tri,triangles);
    soleil.color=vec3(1.,1.,0.);

    Noeud terre;
    loadOFF("../data/sphere.off",terre.id,terre.tri,triangles);
    terre.color=vec3(0.,0.,1.);
    terre.pere=&soleil;
    soleil.enfant.push_back(&terre);

    Noeud lune;
    loadOFF("../data/sphere.off",lune.id,lune.tri,triangles);
    lune.color=vec3(1.,1.,1.);
    lune.pere==&terre;
    terre.enfant.push_back(&lune);

    {
        
    };
   
    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

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
        glfwSetKeyCallback(window, key_callback);


        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glUseProgram(programID);
        glm::vec3 camera_position_used;
        if (cam) {
            camera_position_used = camera_rotate_position;
        } else {
            camera_position_used = camera_free_position;
        }

        if(speed_<2 && sens){
            speed_+=deltaTime*5;
        }else{
            sens=false;
        }

        if(speed_>-2 && !sens){
            speed_-=deltaTime*5;
        }else{
            sens=true;
        }
        

            
            /*

            view = glm::lookAt(camera_position_used, camera_position_used + camera_target, camera_up);
            model = mat4(1.0f);
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); // rotation example
            model = glm::scale(model, glm::vec3(zoom, zoom, zoom));         
            projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            model = translate(model,vec3(0.,speed_,0.));


            set_model(model,test);

            
            set_model(translate(test2.model,vec3(speed_,0.,0.)),test2);
            set_model(translate(test3.model,vec3(-speed_,0.,0.)),test3);

            send(programID,view,projection,test);*/


            angle+=deltaTime*30;

            view = glm::lookAt(camera_position_used, camera_position_used + camera_target, camera_up);
            model = mat4(1.0f);
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); // rotation example
            model = glm::scale(model, glm::vec3(zoom, zoom, zoom));         
            projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);


            set_model(model,soleil);
            soleil.model=glm::scale(soleil.model, glm::vec3(3, 3, 3));

            mat4 model_terre=translate(terre.model,vec3(7,0.,0.));
            model_terre = rotate(model_terre,angle/10.0f, glm::vec3(0,1,0));

            set_model(model_terre,terre);
            terre.model=glm::scale(terre.model, glm::vec3(1, 1, 1));

            mat4 model_lune=translate(terre.model,vec3(2.5,0.,0.));
            set_model(model_lune,lune);
            lune.model=glm::scale(lune.model, glm::vec3(0.2, 0.2, 0.2));

            send(programID,view,projection,soleil);


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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods )
{
    
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        //cout << "touche c utilisé" << endl;
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
}



void processInput(GLFWwindow *window) {
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);



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




// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
