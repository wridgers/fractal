#include <GL/glew.h>
#include <GL/glfw.h>

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

using namespace std;

struct renderConfig {
    int windowWidth;
    int windowHeight;

    int maxIterations;
};

int main(void)
{
    renderConfig *config = new renderConfig;

    config->windowWidth      = 1280;
    config->windowHeight     = 720;
    config->maxIterations    = 100;
    
    // init GLFW
    if (!glfwInit()) {
        cout << "Failed to initialise glfw." << endl;
        return 1;
    }

    // Open OpenGL window
    if (!glfwOpenWindow(config->windowWidth, config->windowHeight, 0, 0, 0, 0, 0, 0, GLFW_WINDOW)) {
        cout << "Failed to create OpenGL window." << endl;
        return 1;
    }

    // now we need to init glew
    if (glewInit() != GLEW_OK) {
        cout << "Failed to initialise glew." << endl;
        return 1;
    }

    // disable v-sync
    glfwSwapInterval(0);

    // set window title to something nice
    glfwSetWindowTitle("GLSL Fractal Renderer");

    // read shader source file
    ifstream shaderFile;
    shaderFile.open("fractal.glsl");

    // check we opened it
    if (!shaderFile.is_open()) {
        cout << "Failed to open shader source." << endl;
        return 1;
    }

    // read contents to a string
    string line, shaderSource;
    while (getline(shaderFile, line))
        shaderSource += line + "\n";

    // get const char ptr
    const char * src = shaderSource.c_str();

    // setup fragment shader
    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    // setup shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, shader);
    glLinkProgram(shaderProgram);

    // current window size
    int currentWidth, currentHeight;

    // frames this second
    int frames = 0;

    // centre of plot
    float re = -0.5, im = 0.0;
    float zoom = 0.2;

    // mouse zoom control
    int oldMouseScroll = glfwGetMouseWheel();
    int newMouseScroll;

    // mouse pan control
    int oldMouseX, newMouseX;
    int oldMouseY, newMouseY;
    glfwGetMousePos(&oldMouseX, &oldMouseY);

    // timer
    chrono::high_resolution_clock::time_point frameStart = chrono::high_resolution_clock::now();

    // render loop
    while(true) {
        // clear buffer
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // draw a fullscreen quad
        glBegin(GL_QUADS);
            glVertex2f(-1, -1);
            glVertex2f(1, -1);
            glVertex2f(1, 1);
            glVertex2f(-1, 1);
        glEnd();

        // use our shader
        glUseProgram(shaderProgram);

        // set quality (iterations)
        GLuint iterationsLocation = glGetUniformLocation(shaderProgram, "iterations");
        glUniform1i(iterationsLocation, config->maxIterations);

        // set zoom
        GLuint zoomLocation = glGetUniformLocation(shaderProgram, "zoom");
        glUniform1f(zoomLocation, zoom);

        // set centre
        GLuint centreLocation = glGetUniformLocation(shaderProgram, "centre");
        glUniform2f(centreLocation, re, im);

        // set resolution variable
        glfwGetWindowSize(&currentWidth, &currentHeight);
        GLuint resolutionLocation = glGetUniformLocation(shaderProgram, "resolution");
        glUniform2f(resolutionLocation, (float)currentWidth, (float)currentHeight);

        // swap buffers
        glfwSwapBuffers();

        // increase frame count
        ++frames;

        // check if one second has passed
        chrono::high_resolution_clock::time_point frameEnd = chrono::high_resolution_clock::now();
        int frameMs = chrono::duration_cast<chrono::milliseconds>(frameEnd - frameStart).count();

        if (frameMs > 1000) {
            cout << "FPS: " << frames << endl;

            frames = 0;
            frameStart = chrono::high_resolution_clock::now();
        }

        // check for mouse zoom (scroll)
        newMouseScroll = glfwGetMouseWheel();
        zoom += (newMouseScroll - oldMouseScroll) * (0.05 * zoom);
        oldMouseScroll = glfwGetMouseWheel();

        // check for mouse pan (move)
        glfwGetMousePos(&newMouseX, &newMouseY);

        // if button is pressed
        if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            // these numbers seem to work well
            re -= (float)(newMouseX - oldMouseX) * (0.001 * (1.0/zoom));
            im += (float)(newMouseY - oldMouseY) * (0.001 * (1.0/zoom));
        }

        // set old position
        glfwGetMousePos(&oldMouseX, &oldMouseY);

        // check for inc/dec iterations
        if (glfwGetKey(GLFW_KEY_KP_ADD) == GLFW_PRESS) config->maxIterations += 1;
        if (glfwGetKey(GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) config->maxIterations -= 1;

        // don't go too low
        if (config->maxIterations < 8) config->maxIterations = 8;

        // space to reset!
        if (glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS) {
            re = -0.5;
            im = 0.0;
            zoom = 0.2;

            config->maxIterations = 100;
        }
        
        // exit if ESC key is pressed
        if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS)
            break;

        // exit if window is closed
        if (glfwGetWindowParam(GLFW_OPENED) == GL_FALSE)
            break;
    }

    // clean up
    delete config;

    // clean up shaders
    glDetachShader(shaderProgram, shader);
    glDeleteShader(shader);

    // clean up glfw
    glfwTerminate();

    // exit
    return 0;
}
