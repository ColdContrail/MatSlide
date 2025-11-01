#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

class Camera {
public:
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 2.0f), 
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
        float yaw = -90.0f, 
        float pitch = 10.0f
    ) : position(position), 
        worldUp(up), 
        yaw(yaw), 
        pitch(pitch), 
        front(glm::vec3(0.0f, 0.0f, -1.0f)), 
        movementSpeed(2.5f), 
        mouseSensitivity(0.1f), 
        fovy(45.0f) 
    {
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 GetProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const {
        return glm::perspective(glm::radians(fovy), aspectRatio, nearPlane, farPlane);
    }

    void Rotate(float deltaYaw, float deltaPitch) {
        yaw += deltaYaw * mouseSensitivity;
        pitch += deltaPitch * mouseSensitivity;
        
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        
        updateCameraVectors();
    }
private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float movementSpeed;
    float mouseSensitivity;
    float fovy;
    void updateCameraVectors() {
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);

        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};