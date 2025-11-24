#include "flock.hpp"
#include <GLFW/glfw3.h> // GLFW
#include <ctime>
#include <iostream>

Flock::Flock() : gen(static_cast<unsigned int>(time(nullptr))), randomInt(0, 1000)
{
    alwaysPerceiveLeader = false;
}

void Flock::add()
{
    bool is_objective = (flock_list.size() == 0);
    // Novos boids usam o estado central do Flock
    Boid b(is_objective, this->alwaysPerceiveLeader);
    flock_list.push_back(b);
}

void Flock::add(glm::vec3 position, glm::vec3 velocity)
{
    bool is_objective = (flock_list.size() == 0);
    // Novos boids usam o estado central do Flock
    Boid b(position, velocity, is_objective, this->alwaysPerceiveLeader);
    flock_list.push_back(b);
}

// deleta um boid aleatório (nunca deleta o líder)
void Flock::clear()
{
    if (flock_list.size() > 1)
    {
        // Gera índice de 1 até size()-1 para nunca pegar o líder (índice 0)
        int randomIndex = 1 + (randomInt(gen) % (flock_list.size() - 1));
        flock_list.erase(flock_list.begin() + randomIndex);
    }
}

void Flock::update(float delta_time, float boundX, float boundY, float boundZ)
{
    for (auto &boid : flock_list)
    {
        boid.update(flock_list, delta_time);
        boid.edges(boundX, boundY, boundZ); // Aplicar colisão com bordas
    }
}

int Flock::size() const
{
    return flock_list.size();
}

std::vector<Boid> &Flock::getBoids()
{
    return flock_list;
}

const std::vector<Boid> &Flock::getBoids() const
{
    return flock_list;
}

void Flock::inputs(GLFWwindow *window)
{
    // processa cada input para movimentação da câmera
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
    {
        this->add();
    }
    
    if (glfwGetKey(window, GLFW_KEY_KP_MULTIPLY) == GLFW_PRESS)
    {
        glm::vec3 flock_center = glm::vec3(0.0f, 0.0f, 0.0f);
        if (this->flock_list.size() > 0)
        {
            for (const Boid &b : this->flock_list)
                flock_center += b.position;
            flock_center /= (float)this->flock_list.size();
        }
        this->add(flock_center, glm::vec3(0.0f, 0.0f, 0.0f));
    }
    
    if (glfwGetKey(window, GLFW_KEY_KP_DIVIDE) == GLFW_PRESS)
    {
        std::uniform_real_distribution<float> randomY(-2.0f, 2.0f);
        std::uniform_real_distribution<float> randomZ(-2.0f, 2.0f);
        this->add(glm::vec3(-35.0f, 50.0f + randomY(gen), randomZ(gen)), glm::vec3(75.0f, 0.0f, 0.0f));
    }
    
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
    {
        this->clear();
        this->clear();
    }
    static bool vKeyWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
    {
        if (!vKeyWasPressed)
        {
            // Toggle do estado central do Flock
            this->alwaysPerceiveLeader = !this->alwaysPerceiveLeader;
            
            // Aplica o novo estado para todos os seguidores
            for (Boid &b : this->flock_list)
            {
                if (!b.isObjective)
                {
                    b.alwaysPerceiveLeader = this->alwaysPerceiveLeader;
                }
            }
            
            std::cout << "Always Perceive Leader: " << (this->alwaysPerceiveLeader ? "ligado" : "desligado") << std::endl;
            vKeyWasPressed = true;
        }
    }
    else
    {
        vKeyWasPressed = false;
    }
}