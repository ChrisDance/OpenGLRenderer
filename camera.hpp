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

    // Fixed camera.hpp - CalculateFrustum method
    void CalculateFrustum(const glm::mat4& projection, const glm::mat4& view)
    {
        glm::mat4 clipMatrix = projection * view;
    
        // Extract matrix elements (GLM is column-major)
        float m00 = clipMatrix[0][0], m01 = clipMatrix[1][0], m02 = clipMatrix[2][0], m03 = clipMatrix[3][0];
        float m10 = clipMatrix[0][1], m11 = clipMatrix[1][1], m12 = clipMatrix[2][1], m13 = clipMatrix[3][1];
        float m20 = clipMatrix[0][2], m21 = clipMatrix[1][2], m22 = clipMatrix[2][2], m23 = clipMatrix[3][2];
        float m30 = clipMatrix[0][3], m31 = clipMatrix[1][3], m32 = clipMatrix[2][3], m33 = clipMatrix[3][3];
    
        // Extract frustum planes (normals point inward)
        // Left plane: w + x >= 0
        ViewFrustum.planes[Frustum::LEFT] = glm::vec4(m03 + m00, m13 + m10, m23 + m20, m33 + m30);
    
        // Right plane: w - x >= 0  
        ViewFrustum.planes[Frustum::RIGHT] = glm::vec4(m03 - m00, m13 - m10, m23 - m20, m33 - m30);
    
        // Bottom plane: w + y >= 0
        ViewFrustum.planes[Frustum::BOTTOM] = glm::vec4(m03 + m01, m13 + m11, m23 + m21, m33 + m31);
    
        // Top plane: w - y >= 0
        ViewFrustum.planes[Frustum::TOP] = glm::vec4(m03 - m01, m13 - m11, m23 - m21, m33 - m31);
    
        // Near plane: w + z >= 0
        ViewFrustum.planes[Frustum::NEAR_PLANE] = glm::vec4(m03 + m02, m13 + m12, m23 + m22, m33 + m32);
    
        // Far plane: w - z >= 0
        ViewFrustum.planes[Frustum::FAR_PLANE] = glm::vec4(m03 - m02, m13 - m12, m23 - m22, m33 - m32);
    
        // Normalize all planes
        for (int i = 0; i < 6; i++) {
            float length = glm::length(glm::vec3(ViewFrustum.planes[i]));
            if (length > 0.0001f) {
                ViewFrustum.planes[i] /= length;
            }
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
