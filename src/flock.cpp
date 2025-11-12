#include "flock.hpp"
#include <GLFW/glfw3.h> // GLFW

Flock::Flock() : gen(rd()), randomInt(0, 1000)
{
}

void Flock::add()
{
    bool is_objective = (flock_list.size() == 0);
    Boid b(is_objective);
    flock_list.push_back(b);
}

// deleta um boid aleatório (nunca deleta o líder)
void Flock::clear()
{
    if (flock_list.size() > 1)
    {
        int randomIndex = randomInt(gen) % (flock_list.size() - 1);
        flock_list.erase(flock_list.begin() + randomIndex + 1);
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
        //for(int i=0; i<100; i++) 
        this->add();
        
    }
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
    {
        this->clear();
        this->clear();
            
    }
}