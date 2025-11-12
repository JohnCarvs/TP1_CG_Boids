#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H
#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>  // GLAD
#include <GLFW/glfw3.h> // GLFW
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>
#include "boid.hpp"
#include "shaderClass.hpp"
#include "flock.hpp"
#include "boid.hpp"


class Camera{
    public:
        glm::vec3 Position;
        glm::vec3 Orientation = glm::vec3(0.0, 0.0, -1.0f);
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 cameraMatrix = glm::mat4(1.0f);
        
        glm::vec3 prevPosition;
        glm::vec3 prevOrientation = glm::vec3(0.0, 0.0, 1.0f);
        
        bool followCenterMode = false;  // Torre no centro
        bool followLeaderMode = false;  // Atrás do bando
        bool perpendicularMode = false; // Perpendicular à velocidade
        bool firstClick = true;
        
        // Suavização de câmera
        float cameraSmoothness = 0.15f; // menor = mais suave
        
        int width;
        int height;

        float speed = 20.0f;
        float sensitivity = 100.0f;
        
        // Para seguir boid
        const Boid* targetBoid = nullptr;
        glm::vec3 followOffset = glm::vec3(0.0f, 5.0f, -15.0f);
        float followSmoothness = 0.1f;

        Camera(int width, int height, glm::vec3 position);

        void updateMatrix(float FOVdeg, float nearPlane, float farPlane, const Flock& flock);  
        void Matrix(Shader& shader, const char* uniform);
        void Inputs(GLFWwindow* window, float deltaTime);

    };


#endif