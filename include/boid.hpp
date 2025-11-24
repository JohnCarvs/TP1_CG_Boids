#ifndef BOID_CLASS_H
#define BOID_CLASS_H

#include <glm/glm.hpp>
#include <vector>

// Estrutura para representar obstáculos (árvores)
struct Tree {
    glm::vec3 position;
    float radius;  // Raio da árvore
    float height;  // Altura da árvore
};

class Boid {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    // Parâmetros de comportamento
    float maxSpeed;
    float maxForce;
    float perceptionRadius;
    bool isObjective;
    bool alwaysPerceiveLeader;

    // Animação das asas
    float wingPhase;       // Fase atual [0, 2pi]
    float wingFrequency;   // Frequencia de batida

    // construtor padrao
    Boid(glm::vec3 pos, glm::vec3 vel, bool objective = false, bool alwaysPerceiveLeader = false);

    // construtor vazio para aleatoriedade
    Boid(bool objective = false, bool alwaysPerceiveLeader = false);
    
    void update(std::vector<Boid> flock_list, float delta_time);
    void applyForce(glm::vec3 force);
    void flock(const std::vector<Boid>& boids);
    
    // Regras dos boids
    glm::vec3 separation(const std::vector<Boid>& boids);
    glm::vec3 alignment(const std::vector<Boid>& boids);
    glm::vec3 cohesion(const std::vector<Boid>& boids);
    glm::vec3 objective(const std::vector<Boid>& boids);
    glm::vec3 avoidObstacles(const std::vector<Tree>& trees);
    
    // Função auxiliar para buscar um alvo
    glm::vec3 seek(glm::vec3 target);
    
    // Manter dentro dos limites
    void edges(float boundX, float boundY, float boundZ);
    
    // Transformação para renderização
    glm::mat4 getModelMatrix() const;
};

#endif
