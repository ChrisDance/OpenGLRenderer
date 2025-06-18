#pragma once
#include "mygl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Frustum {
    glm::vec4 planes[6]; // left, right, bottom, top, near, far

    enum PlaneIndex {
        LEFT = 0,
        RIGHT = 1,
        BOTTOM = 2,
        TOP = 3,
        NEAR_PLANE = 4,
        FAR_PLANE = 5
    };
};

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    Frustum ViewFrustum;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(10.5f), MouseSensitivity(0.1f), Zoom(45.0f)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    void CalculateFrustum(const glm::mat4& projection, const glm::mat4& view)
    {
        glm::mat4 clipMatrix = projection * view;

        // Extract frustum planes from clip matrix
        // Left plane
        ViewFrustum.planes[Frustum::LEFT] = glm::vec4(
            clipMatrix[0][3] + clipMatrix[0][0],
            clipMatrix[1][3] + clipMatrix[1][0],
            clipMatrix[2][3] + clipMatrix[2][0],
            clipMatrix[3][3] + clipMatrix[3][0]
        );

        // Right plane
        ViewFrustum.planes[Frustum::RIGHT] = glm::vec4(
            clipMatrix[0][3] - clipMatrix[0][0],
            clipMatrix[1][3] - clipMatrix[1][0],
            clipMatrix[2][3] - clipMatrix[2][0],
            clipMatrix[3][3] - clipMatrix[3][0]
        );

        // Bottom plane
        ViewFrustum.planes[Frustum::BOTTOM] = glm::vec4(
            clipMatrix[0][3] + clipMatrix[0][1],
            clipMatrix[1][3] + clipMatrix[1][1],
            clipMatrix[2][3] + clipMatrix[2][1],
            clipMatrix[3][3] + clipMatrix[3][1]
        );

        // Top plane
        ViewFrustum.planes[Frustum::TOP] = glm::vec4(
            clipMatrix[0][3] - clipMatrix[0][1],
            clipMatrix[1][3] - clipMatrix[1][1],
            clipMatrix[2][3] - clipMatrix[2][1],
            clipMatrix[3][3] - clipMatrix[3][1]
        );

        // Near plane
        ViewFrustum.planes[Frustum::NEAR_PLANE] = glm::vec4(
            clipMatrix[0][3] + clipMatrix[0][2],
            clipMatrix[1][3] + clipMatrix[1][2],
            clipMatrix[2][3] + clipMatrix[2][2],
            clipMatrix[3][3] + clipMatrix[3][2]
        );

        // Far plane
        ViewFrustum.planes[Frustum::FAR_PLANE] = glm::vec4(
            clipMatrix[0][3] - clipMatrix[0][2],
            clipMatrix[1][3] - clipMatrix[1][2],
            clipMatrix[2][3] - clipMatrix[2][2],
            clipMatrix[3][3] - clipMatrix[3][2]
        );

        // Normalize all planes
        for (int i = 0; i < 6; i++) {
            float length = glm::length(glm::vec3(ViewFrustum.planes[i]));
            ViewFrustum.planes[i] /= length;
        }
    }
    
    
    
    

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(int direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == 0) // FORWARD
            Position += Front * velocity;
        if (direction == 1) // BACKWARD
            Position -= Front * velocity;
        if (direction == 2) // LEFT
            Position -= Right * velocity;
        if (direction == 3) // RIGHT
            Position += Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
