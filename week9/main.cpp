#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <iostream>

namespace {
constexpr int kWidth = 1000, kHeight = 700;
glm::vec3 camPos(0.0f, 0.0f, 3.0f);
glm::vec3 camFront(0.0f, 0.0f, -1.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);
bool fixedTarget = false, resetWasDown = false, modeWasDown = false;

const char* vertexSource = R"GLSL(#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aColor;
out vec3 vertexColor;
uniform mat4 model, view, projection;
void main() {
    vertexColor = aColor;
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
})GLSL";

const char* fragmentSource = R"GLSL(#version 330 core
in vec3 vertexColor;
out vec4 color;
void main() { color = vec4(vertexColor, 1.0); }
)GLSL";

GLuint compile(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compilation failed: " << log << '\n';
    }
    return shader;
}
GLuint makeProgram() {
    GLuint vs = compile(GL_VERTEX_SHADER, vertexSource);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs); glAttachShader(program, fs); glLinkProgram(program);
    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Program link failed: " << log << '\n';
    }
    glDeleteShader(vs); glDeleteShader(fs);
    return program;
}
void onResize(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

void processInput(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    bool reset = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if (reset && !resetWasDown) camPos = {0.0f, 0.0f, 3.0f};
    resetWasDown = reset;
    bool mode = glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS;
    if (mode && !modeWasDown) fixedTarget = !fixedTarget;
    modeWasDown = mode;

    float speed = 2.5f * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) speed *= 2.0f;
    glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += speed * camFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= speed * camFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= speed * right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += speed * right;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camPos += speed * camUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camPos -= speed * camUp;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        std::cout << "Camera: (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")\n";
}
} // namespace

int main() {
    if (!glfwInit()) { std::cerr << "GLFW initialization failed\n"; return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(kWidth, kHeight, "Week 9 - Camera", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "GLAD initialization failed\n";
        glfwDestroyWindow(window); glfwTerminate(); return 1;
    }
    glViewport(0, 0, kWidth, kHeight);
    glEnable(GL_DEPTH_TEST);

    // 36 vertices with per-face colors.
    constexpr std::array<float, 216> cube = {
        -.5f,-.5f,-.5f, 1,.15f,.15f, .5f,.5f,-.5f, 1,.15f,.15f, .5f,-.5f,-.5f, 1,.15f,.15f,
        .5f,.5f,-.5f, 1,.15f,.15f, -.5f,-.5f,-.5f, 1,.15f,.15f, -.5f,.5f,-.5f, 1,.15f,.15f,
        -.5f,-.5f,.5f, .15f,1,.25f, .5f,-.5f,.5f, .15f,1,.25f, .5f,.5f,.5f, .15f,1,.25f,
        .5f,.5f,.5f, .15f,1,.25f, -.5f,.5f,.5f, .15f,1,.25f, -.5f,-.5f,.5f, .15f,1,.25f,
        -.5f,.5f,.5f, .2f,.4f,1, -.5f,.5f,-.5f, .2f,.4f,1, -.5f,-.5f,-.5f, .2f,.4f,1,
        -.5f,-.5f,-.5f, .2f,.4f,1, -.5f,-.5f,.5f, .2f,.4f,1, -.5f,.5f,.5f, .2f,.4f,1,
        .5f,.5f,.5f, 1,.85f,.15f, .5f,-.5f,-.5f, 1,.85f,.15f, .5f,.5f,-.5f, 1,.85f,.15f,
        .5f,-.5f,-.5f, 1,.85f,.15f, .5f,.5f,.5f, 1,.85f,.15f, .5f,-.5f,.5f, 1,.85f,.15f,
        -.5f,-.5f,-.5f, .15f,.9f,.9f, .5f,-.5f,-.5f, .15f,.9f,.9f, .5f,-.5f,.5f, .15f,.9f,.9f,
        .5f,-.5f,.5f, .15f,.9f,.9f, -.5f,-.5f,.5f, .15f,.9f,.9f, -.5f,-.5f,-.5f, .15f,.9f,.9f,
        -.5f,.5f,-.5f, .8f,.25f,1, .5f,.5f,.5f, .8f,.25f,1, .5f,.5f,-.5f, .8f,.25f,1,
        .5f,.5f,.5f, .8f,.25f,1, -.5f,.5f,-.5f, .8f,.25f,1, -.5f,.5f,.5f, .8f,.25f,1
    };
    constexpr std::array<glm::vec3, 5> positions = {
        glm::vec3(0,0,0), glm::vec3(2,.5f,-2), glm::vec3(-1.8f,-.3f,-1),
        glm::vec3(1.2f,-.8f,-3.5f), glm::vec3(-2.2f,1,-4)
    };

    GLuint vao=0, vbo=0;
    glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
    glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(cube),cube.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),reinterpret_cast<void*>(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint program=makeProgram();
    GLint modelLoc=glGetUniformLocation(program,"model");
    GLint viewLoc=glGetUniformLocation(program,"view");
    GLint projectionLoc=glGetUniformLocation(program,"projection");
    float lastFrame=static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window)) {
        float now=static_cast<float>(glfwGetTime());
        float dt=now-lastFrame; lastFrame=now;
        processInput(window,dt);
        glClearColor(.07f,.08f,.12f,1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        // M compares the fixed world origin with the camera-relative look direction.
        glm::vec3 target=fixedTarget ? glm::vec3(0) : camPos+camFront;
        glm::mat4 view=glm::lookAt(camPos,target,camUp);
        int w=0,h=0; glfwGetFramebufferSize(window,&w,&h);
        float aspect=h>0 ? static_cast<float>(w)/h : 1.0f;
        glm::mat4 projection=glm::perspective(glm::radians(45.0f),aspect,.1f,100.0f);
        glUniformMatrix4fv(viewLoc,1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc,1,GL_FALSE,glm::value_ptr(projection));
        glBindVertexArray(vao);
        for (std::size_t i=0;i<positions.size();++i) {
            glm::mat4 model(1);
            model=glm::translate(model,positions[i]);
            float angle=glm::radians(18.0f*static_cast<float>(i+1))*now;
            model=glm::rotate(model,angle,glm::normalize(glm::vec3(.3f,1,0)));
            glUniformMatrix4fv(modelLoc,1,GL_FALSE,glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES,0,36);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteProgram(program); glDeleteBuffers(1,&vbo); glDeleteVertexArrays(1,&vao);
    glfwDestroyWindow(window); glfwTerminate();
    return 0;
}
