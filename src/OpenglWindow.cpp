#include "OpenglWindow.hpp"

void OpenglWindow::onMouseMoved(GLFWwindow*, double xpos, double ypos) {
}
void OpenglWindow::onScrollOccurred(GLFWwindow*, double, double) {
}
void OpenglWindow::processInput() {
}
void OpenglWindow::initGlfw() {
    glfwInit();
    using namespace std::placeholders;
    window_ = glfwCreateWindow(width_, height_, win_title_.c_str(), NULL, NULL);

    glfwMakeContextCurrent(window_);
    gl3wInit();


    auto mouse_callback = std::function<void(GLFWwindow*, double, double)>(
                              std::bind(&OpenglWindow::onMouseMoved, this, _1, _2, _3))
                              .target<void(GLFWwindow*, double, double)>();
    auto scroll_callback = std::function<void(GLFWwindow*, double, double)>(
                               std::bind(&OpenglWindow::onScrollOccurred, this, _1, _2, _3))
                               .target<void(GLFWwindow*, double, double)>();
    glfwSetScrollCallback(window_, scroll_callback);
    glfwSetCursorPosCallback(window_, mouse_callback);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glDebugMessageCallback(glDebugOutput, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
}
void OpenglWindow::init() {
}
void OpenglWindow::display() {
}
void OpenglWindow::show() {
    initGlfw();
    init();
    while (!glfwWindowShouldClose(window_)) {
        processInput();
        display();
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}
OpenglWindow::~OpenglWindow() {
    glfwDestroyWindow(window_);
    glfwTerminate();
}