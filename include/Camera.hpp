#ifndef INCLUDE_CAMERA
#define INCLUDE_CAMERA
#include <functional>
#if _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <windows.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
// #include <GL3/gl3.h>
// #include <GL3/gl3w.h>
#include <GLFW/glfw3.h>
#include <vector>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
// An abstract camera class that processes input and calculates the corresponding Euler Angles,
// Vectors and Matrices for use in OpenGL
class Camera {
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    bool fps_ = false;
    bool use_my_look_at_ = false;
    std::function<glm::mat4(glm::vec3, glm::vec3, glm::vec3)> lookAt =
        [](glm::vec3 pos, glm::vec3 center, glm::vec3 up) {
            auto cameraDirection = glm::normalize(pos - center);
            auto right = glm::normalize(glm::cross(cameraDirection, up));
            auto newUp = glm::cross(cameraDirection, right);
            return glm::mat4(right.x,
                             right.y,
                             right.z,
                             0,
                             newUp.x,
                             newUp.y,
                             newUp.z,
                             0,
                             cameraDirection.x,
                             cameraDirection.y,
                             cameraDirection.z,
                             0,
                             0,
                             0,
                             0,
                             1) *
                   glm::mat4(1, 0, 0, -pos.x, 0, 1, 0, -pos.y, 0, 0, 1, -pos.z, 0, 0, 0, 1);
        };

    void useMyLookAt(bool use_my_look_at) {
        use_my_look_at_ = use_my_look_at;
    }
    void setFpsCamera(bool fps) {
        fps_ = fps;
    }

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW,
           float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY),
          Zoom(ZOOM) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX,
           float posY,
           float posZ,
           float upX,
           float upY,
           float upZ,
           float yaw,
           float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY),
          Zoom(ZOOM) {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix() {
        if (use_my_look_at_){
            lookAt(Position, Position + Front, Up);
        }
        return glm::lookAt(Position, Position + Front, Up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in
    // the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (fps_) {
            auto cur_front = glm::vec3(Front.x, 0, Front.z);
            auto cur_right = glm::vec3(Right.x, 0, Right.z);

            if (direction == FORWARD)
                Position += cur_front * velocity;
            if (direction == BACKWARD)
                Position -= cur_front * velocity;
            if (direction == LEFT)
                Position -= cur_right * velocity;
            if (direction == RIGHT)
                Position += cur_right * velocity;
        } else {
            if (direction == FORWARD)
                Position += Front * velocity;
            if (direction == BACKWARD)
                Position -= Front * velocity;
            if (direction == LEFT)
                Position -= Right * velocity;
            if (direction == RIGHT)
                Position += Right * velocity;
        }
    }

    // processes input received from a mouse input system. Expects the offset value in both the
    // x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the
    // vertical wheel-axis
    void ProcessMouseScroll(float yoffset) {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors() {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(
            Front,
            WorldUp));  // normalize the vectors, because their length gets closer to 0 the
                        // more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif /* INCLUDE_CAMERA */
