#ifndef BOID_CLASS_H
#define BOID_CLASS_H

#include <glm/glm.hpp>
#include <vector>

class Boid {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    // Parâmetros de comportamento
    float maxSpeed;
    float maxForce;
    float perceptionRadius;

    Boid(glm::vec3 pos, glm::vec3 vel);
    
    void update(float deltaTime);
    void applyForce(glm::vec3 force);
    void flock(const std::vector<Boid>& boids);
    
    // Regras dos boids
    glm::vec3 separation(const std::vector<Boid>& boids);
    glm::vec3 alignment(const std::vector<Boid>& boids);
    glm::vec3 cohesion(const std::vector<Boid>& boids);
    
    // Manter dentro dos limites
    void edges(float boundX, float boundY, float boundZ);
    
    // Transformação para renderização
    glm::mat4 getModelMatrix() const;
};

#endif
