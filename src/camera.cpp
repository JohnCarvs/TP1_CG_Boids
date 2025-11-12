#include "camera.hpp"

Camera::Camera(int width, int height, glm::vec3 position)
{
    Camera::width = width;
    Camera::height = height;
    Position = position;
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane, const Flock &flock)
{
    // Calcular centro do bando
    glm::vec3 flock_center = glm::vec3(0.0f, 0.0f, 0.0f);
    if (flock.size() > 0)
    {
        for (const Boid& b : flock.getBoids())
            flock_center += b.position;
        flock_center /= (float)flock.size();
    }
    
    // Calcular velocidade média do bando
    glm::vec3 flock_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    if (flock.size() > 0)
    {
        for (const Boid& b : flock.getBoids())
            flock_velocity += b.velocity;
        flock_velocity /= (float)flock.size();
    }

    // MODO 1: Torre no centro (posição alta e fixa, olhando para o centro do bando)
    if (followCenterMode && flock.size() > 0)
    {
        // Posição alvo no alto da "torre" no centro do mundo (0, altura, 0)
        glm::vec3 targetPosition = glm::vec3(0.0f, 60.0f, 0.0f);
        // Orientação alvo aponta do olho para o centro do bando
        glm::vec3 targetOrientation = glm::normalize(flock_center - targetPosition);
        
        // Suavização (lerp)
        Position = glm::mix(Position, targetPosition, cameraSmoothness);
        Orientation = glm::normalize(glm::mix(Orientation, targetOrientation, cameraSmoothness));
    }
    
    // MODO 2: Atrás do bando a distância fixa
    else if (followLeaderMode && flock.size() > 0 && glm::length(flock_velocity) > 0.1f)
    {
        // Direção oposta à velocidade do bando
        glm::vec3 behind_direction = glm::normalize(-flock_velocity);
        // Posição alvo atrás do centro do bando a distância fixa
        float distance = 40.0f;
        glm::vec3 targetPosition = flock_center + behind_direction * distance;
        // Manter altura mínima
        if (targetPosition.y < flock_center.y + 5.0f)
            targetPosition.y = flock_center.y + 5.0f;
        // Orientação alvo aponta para o centro do bando
        glm::vec3 targetOrientation = glm::normalize(flock_center - targetPosition);
        
        // Suavização (lerp)
        Position = glm::mix(Position, targetPosition, cameraSmoothness);
        Orientation = glm::normalize(glm::mix(Orientation, targetOrientation, cameraSmoothness));
    }
    
    // MODO 3: Perpendicular ao vetor velocidade, paralelo ao chão
    else if (perpendicularMode && flock.size() > 0 && glm::length(flock_velocity) > 0.1f)
    {
        // Projetar velocidade no plano XZ (remover componente Y)
        glm::vec3 velocity_ground = glm::vec3(flock_velocity.x, 0.0f, flock_velocity.z);
        
        if (glm::length(velocity_ground) > 0.1f)
        {
            velocity_ground = glm::normalize(velocity_ground);
            
            // Vetor perpendicular à velocidade no plano do chão (rotação de 90 no plano XZ)
            glm::vec3 perpendicular = glm::vec3(-velocity_ground.z, 0.0f, velocity_ground.x);
            
            // Posição alvo perpendicular ao movimento, na altura do centro do bando
            float distance = 40.0f;
            glm::vec3 targetPosition = flock_center + perpendicular * distance;
            targetPosition.y = flock_center.y; // Mesma altura que o centro do bando
            
            // Orientação alvo aponta para o centro do bando
            glm::vec3 targetOrientation = glm::normalize(flock_center - targetPosition);
            
            // Suavização (lerp)
            Position = glm::mix(Position, targetPosition, cameraSmoothness);
            Orientation = glm::normalize(glm::mix(Orientation, targetOrientation, cameraSmoothness));
        }
    }

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // configura matrizes de visualização e projeção
    view = glm::lookAt(Position, Position + Orientation, Up);
    
    // Calcular aspect ratio corretamente (evitar divisão por zero)
    float aspect = (height > 0) ? (float)width / (float)height : 1.0f;
    projection = glm::perspective(glm::radians(FOVdeg), aspect, nearPlane, farPlane);

    cameraMatrix = projection * view;
}

void Camera::Matrix(Shader &shader, const char *uniform)
{
    // envia as matrizes para o shader
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::Inputs(GLFWwindow *window, float deltaTime)
{
    // MODO 1: Torre no centro (olhando para o bando do alto)
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        if (!this->followCenterMode && !this->followLeaderMode && !this->perpendicularMode)
        {
            this->prevPosition = Position;
            this->prevOrientation = Orientation;
        }
        this->followCenterMode = true;
        this->followLeaderMode = false;
        this->perpendicularMode = false;
    }
    
    // MODO 2: Atrás do bando a distância fixa
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        if (!this->followCenterMode && !this->followLeaderMode && !this->perpendicularMode)
        {
            this->prevPosition = Position;
            this->prevOrientation = Orientation;
        }
        this->followLeaderMode = true;
        this->followCenterMode = false;
        this->perpendicularMode = false;
    }
    // Tecla 3: Modo 3 - Perpendicular à velocidade do bando, paralelo ao chão
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        if (!this->followCenterMode && !this->followLeaderMode && !this->perpendicularMode)
        {
            this->prevPosition = Position;
            this->prevOrientation = Orientation;
        }
        this->perpendicularMode = true;
        this->followCenterMode = false;
        this->followLeaderMode = false;
    }
    // Tecla 4: Retornar ao modo de navegação livre
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        this->Position = prevPosition;
        this->Orientation = prevOrientation;
        this->followCenterMode = false;
        this->followLeaderMode = false;
        this->perpendicularMode = false;
    }
    if (followCenterMode || followLeaderMode || perpendicularMode)
        return;

    // processa cada input para movimentação da câmera
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Position += speed * deltaTime * Orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        Position += speed * deltaTime * -glm::normalize(glm::cross(Orientation, Up));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        Position += speed * deltaTime * -Orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        Position += speed * deltaTime * glm::normalize(glm::cross(Orientation, Up));
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        Position += speed * deltaTime * Up;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        Position += speed * deltaTime * -Up;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        speed = 40.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
    {
        speed = 20.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        std::cout << "Camera Position: [" << Position.x << " , " << Position.y << " , " << Position.z << "]" << std::endl;
        std::cout << "Camera Orientation: [" << Orientation.x << " , " << Orientation.y << " , " << Orientation.z << "]" << std::endl;
    }

    // processa inputs do mouse
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (firstClick)
        {
            glfwSetCursorPos(window, (width / 2), (height / 2));
            firstClick = false;
        }

        double mouseX;
        double mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float rotx = sensitivity * (float)(mouseY - (height / 2)) / height;
        float roty = sensitivity * (float)(mouseX - (width / 2)) / width;

        glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotx), glm::normalize(glm::cross(Orientation, Up)));

        if (!((glm::angle(newOrientation, Up) <= glm::radians(5.0)) or (glm::angle(newOrientation, -Up) <= glm::radians(5.0f))))
        {
            Orientation = newOrientation;
        }

        Orientation = glm::rotate(Orientation, glm::radians(-roty), Up);

        glfwSetCursorPos(window, (width / 2), (height / 2));
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }
}
