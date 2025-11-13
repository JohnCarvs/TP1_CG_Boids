#ifndef FLOCK_CLASS_H
#define FLOCK_CLASS_H

#include <glm/glm.hpp>
#include <vector>
#include "boid.hpp"
#include <random>
#include <GLFW/glfw3.h> // GLFW


class Flock
{
private:
    std::vector<Boid> flock_list;

    std::mt19937 gen;
    std::uniform_int_distribution<int> randomInt;

public:
    Flock();
    
    void add();

    // deleta um boid aleatório
    void clear();

    void update(float delta_time, float boundX, float boundY, float boundZ);

    int size() const;

    // inputs
    void inputs(GLFWwindow *window);
    
    // Acessar boids para renderização
    std::vector<Boid>& getBoids();
    const std::vector<Boid>& getBoids() const;
};

#endif
