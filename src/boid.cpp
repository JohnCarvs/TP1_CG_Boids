#include "boid.hpp"
#include <random>
#include <glm/gtc/matrix_transform.hpp>

Boid::Boid(glm::vec3 pos, glm::vec3 vel, bool objective, bool alwaysPerceiveLeader)
{
    position = pos;
    velocity = vel;
    acceleration = glm::vec3(0.0f);
    maxSpeed = 35.0f;
    maxForce = 0.5f;
    perceptionRadius = 50.0f;
    isObjective = objective;
    alwaysPerceiveLeader = alwaysPerceiveLeader;
    
    // Inicializar animação das asas com fase aleatória baseada no endereço do objeto
    wingPhase = static_cast<float>(reinterpret_cast<uintptr_t>(this) % 628) / 100.0f; // 0 a 6.28
    wingFrequency = 5.0f;  // 5 batidas por segundo
}

Boid::Boid(bool objective, bool alwaysPerceiveLeader)
{
    // Usar endereço do objeto como seed (único para cada boid)
    std::mt19937 gen(reinterpret_cast<uintptr_t>(this));
    std::uniform_real_distribution<float> randomPos(-5.0, 5.0);
    std::uniform_real_distribution<float> randomVel(-1.0, 1.0);
        
    position = glm::vec3(randomPos(gen), randomPos(gen), randomPos(gen));
    velocity = glm::vec3(randomVel(gen), randomVel(gen), randomVel(gen));
    acceleration = glm::vec3(0.0f);
    maxSpeed = 75.0f;
    maxForce = 0.5f;
    perceptionRadius = 50.0f;
    isObjective = objective;
    alwaysPerceiveLeader = alwaysPerceiveLeader;
    
    // Inicializar animação das asas com fase aleatória baseada no endereço do objeto
    wingPhase = static_cast<float>(reinterpret_cast<uintptr_t>(this) % 628) / 100.0f; // 0 a 6.28
    wingFrequency = 5.0f;  // 5 batidas por segundo
}

// Árvores globais
extern std::vector<Tree> globalTrees;

void Boid::update(std::vector<Boid> flock_list, float delta_time)
{
    // Aplicar comportamentos de bando
    flock(flock_list);
    
    // Aplicar comportamento de objetivo (seguir líder)
    if (!isObjective)
    {
        glm::vec3 objective_update = objective(flock_list);
        objective_update *= 1.5f;  // Peso para seguir o líder
        applyForce(objective_update);
    }
    
    // Evitar obstáculos
    glm::vec3 obstacleAvoidance = avoidObstacles(globalTrees);
    obstacleAvoidance *= 30.0f;  // Peso 
    applyForce(obstacleAvoidance);
    
    // Atualizar velocidade
    velocity += acceleration;
    
    // Limitar a velocidade máxima
    if (glm::length(velocity) > maxSpeed)
    {
        velocity = glm::normalize(velocity) * maxSpeed;
    }
    
    // Atualizar posição
    position += velocity * delta_time;
    
    // Atualizar animação das asas
    wingPhase += wingFrequency * 2.0f * 3.14159265f * delta_time;
    if (wingPhase > 2.0f * 3.14159265f)
        wingPhase -= 2.0f * 3.14159265f;
    
    // Resetar aceleração para o próximo frame
    acceleration = glm::vec3(0.0f);
}

void Boid::applyForce(glm::vec3 force)
{
    acceleration += force;
}

void Boid::flock(const std::vector<Boid>& boids)
{
    glm::vec3 sep = separation(boids);
    glm::vec3 ali = alignment(boids);
    glm::vec3 coh = cohesion(boids);
    
    // Pesos para cada força
    sep *= 2.0f;  
    ali *= 0.4f;  
    coh *= 0.4f;  

    applyForce(sep);
    applyForce(ali);
    applyForce(coh);
}

// separacao - evitar colisão com vizinhos próximos
glm::vec3 Boid::separation(const std::vector<Boid> &boids)
{
    float desiredSeparation = 25.0f;
    glm::vec3 steer = glm::vec3(0.0f);
    int count = 0;
    
    for (const auto& other : boids)
    {
        float d = glm::distance(position, other.position);
        
        // Se ta muito perto, mas não é ele mesmo
        if (d > 0.0f && d < desiredSeparation)
        {
            // Calcular vetor apontando para longe do vizinho
            glm::vec3 diff = position - other.position;
            diff = glm::normalize(diff);
            diff /= d;  // Quanto mais perto, maior a força
            steer += diff;
            count++;
        }
    }
    
    // Média dos vetores de afastamento
    if (count > 0)
    {
        steer /= (float)count;
    }
    
    // Implementar steering (Reynolds)
    if (glm::length(steer) > 0.0f)
    {
        steer = glm::normalize(steer) * maxSpeed;
        steer -= velocity;
        if (glm::length(steer) > maxForce)
        {
            steer = glm::normalize(steer) * maxForce;
        }
    }
    
    return steer;
}

// Regra 2: ALINHAMENTO - Alinhar com a direção média dos vizinhos
glm::vec3 Boid::alignment(const std::vector<Boid> &boids)
{
    glm::vec3 sum = glm::vec3(0.0f);
    int count = 0;
    
    for (const auto& other : boids)
    {
        float d = glm::distance(position, other.position);
        
        if (d > 0.0f && d < perceptionRadius)
        {
            sum += other.velocity;
            count++;
        }
    }
    
    if (count > 0)
    {
        sum /= (float)count;
        sum = glm::normalize(sum) * maxSpeed;
        
        glm::vec3 steer = sum - velocity;
        if (glm::length(steer) > maxForce)
        {
            steer = glm::normalize(steer) * maxForce;
        }
        return steer;
    }
    
    return glm::vec3(0.0f);
}

// Regra 3: COESÃO - Mover em direção à posição média dos vizinhos
glm::vec3 Boid::cohesion(const std::vector<Boid> &boids)
{
    glm::vec3 sum = glm::vec3(0.0f);
    int count = 0;
    
    for (const auto& other : boids)
    {
        float d = glm::distance(position, other.position);
        
        if (d > 0.0f && d < perceptionRadius)
        {
            sum += other.position;
            count++;
        }
    }
    
    if (count > 0)
    {
        sum /= (float)count;
        return seek(sum);  // Buscar a posição média
    }
    
    return glm::vec3(0.0f);
}

// Função auxiliar: buscar um alvo
glm::vec3 Boid::seek(glm::vec3 target)
{
    glm::vec3 desired = target - position;
    desired = glm::normalize(desired) * maxSpeed;
    
    glm::vec3 steer = desired - velocity;
    if (glm::length(steer) > maxForce)
    {
        steer = glm::normalize(steer) * maxForce;
    }
    
    return steer;
}

// Regra especial: OBJETIVO - Seguir o boid [0] (líder do bando)
glm::vec3 Boid::objective(const std::vector<Boid> &boids)
{
    if (isObjective || boids.empty())
    {
        return glm::vec3(0.0f);  // O objetivo não segue ninguém
    }
    
    // Seguir o boid [0] (líder)
    const Boid& leader = boids[0];
    
    glm::vec3 desired = leader.position - position;
    float d = glm::length(desired);
    
    if (d > 0.0f && (d < 100.0f || this->alwaysPerceiveLeader))
    {
        desired = glm::normalize(desired) * maxSpeed * 0.5f;  // 50% da velocidade
        
        glm::vec3 steer = desired - velocity;
        if (glm::length(steer) > maxForce * 0.8f)
        {
            steer = glm::normalize(steer) * maxForce * 0.8f;
        }
        
        return steer;
    }
    
    return glm::vec3(0.0f);
}

void Boid::edges(float boundX, float boundY, float boundZ)
{
    float margin = 30.0f;  // Distância da borda onde começa a repulsão
    float turnFactor = 10.0f;  // Força suave de afastamento
    
    // Aplicar força de repulsão gradual quando está perto das bordas
    // Quanto mais perto da borda, maior a força
    
    // Borda direita (X positivo)
    if (position.x > boundX - margin)
    {
        float distance = boundX - position.x;
        float force = turnFactor * (1.0f - distance / margin);
        velocity.x -= force;
    }
    // Borda esquerda (X negativo)
    if (position.x < -boundX + margin)
    {
        float distance = position.x + boundX;
        float force = turnFactor * (1.0f - distance / margin);
        velocity.x += force;
    }
    
    // Borda superior (Y positivo)
    if (position.y > boundY - margin)
    {
        float distance = boundY - position.y;
        float force = turnFactor * (1.0f - distance / margin);
        velocity.y -= force;
    }
    // Borda inferior (Y ~ chão)
    if (position.y < 10.0f + margin)
    {
        float distance = position.y - 10.0f;
        float force = turnFactor * (1.0f - distance / margin);
        velocity.y += force;
    }
    
    // Borda frente (Z positivo)
    if (position.z > boundZ - margin)
    {
        float distance = boundZ - position.z;
        float force = turnFactor * (1.0f - distance / margin);
        velocity.z -= force;
    }
    // Borda trás (Z negativo)
    if (position.z < -boundZ + margin)
    {
        float distance = position.z + boundZ;
        float force = turnFactor * (1.0f - distance / margin);
        velocity.z += force;
    }
    
    // Garantir que não ultrapasse (caso a força não seja suficiente)
    position.x = glm::clamp(position.x, -boundX, boundX);
    position.y = glm::clamp(position.y, 10.0f, boundY);
    position.z = glm::clamp(position.z, -boundZ, boundZ);
}

glm::vec3 Boid::avoidObstacles(const std::vector<Tree>& trees)
{
    float detectionRadius = 15.0f;  // Distância de detecção
    glm::vec3 steer = glm::vec3(0.0f);
    int count = 0;
    
    for (const auto& tree : trees)
    {
        // Calcular distância 2D (XZ) e verificar altura
        glm::vec2 posXZ = glm::vec2(position.x, position.z);
        glm::vec2 treeXZ = glm::vec2(tree.position.x, tree.position.z);
        float distXZ = glm::distance(posXZ, treeXZ);
        
        // Verificar se está na altura da árvore
        bool inHeightRange = (position.y >= tree.position.y - 1000.0f) && 
                             (position.y <= tree.position.y + tree.height + 1000.0f);
        
        if (distXZ < detectionRadius && inHeightRange)
        {
            // Calcular vetor de repulsão
            glm::vec3 diff = position - tree.position;
            float distance = glm::length(diff);
            
            if (distance > 0.0f)
            {
                diff = glm::normalize(diff);
                // Força inversamente proporcional à distância
                diff /= (distance * distance);
                steer += diff;
                count++;
            }
        }
    }
    
    if (count > 0)
    {
        steer /= static_cast<float>(count);
        
        if (glm::length(steer) > 0.0f)
        {
            steer = glm::normalize(steer) * maxSpeed;
            steer -= velocity;
            
            if (glm::length(steer) > maxForce)
            {
                steer = glm::normalize(steer) * maxForce;
            }
        }
    }
    
    return steer;
}

glm::mat4 Boid::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    
    // Orientar o boid na direção da velocidade
    if (glm::length(velocity) > 0.001f)
    {
        glm::vec3 forward = glm::normalize(velocity);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        
        // Evitar problema quando forward é paralelo a up
        if (abs(glm::dot(forward, up)) > 0.99f)
        {
            up = glm::vec3(0.0f, 0.0f, 1.0f);
        }
        
        glm::vec3 right = glm::normalize(glm::cross(forward, up));
        up = glm::cross(right, forward);
        
        glm::mat4 rotation = glm::mat4(1.0f);
        rotation[0] = glm::vec4(right, 0.0f);
        rotation[1] = glm::vec4(up, 0.0f);
        rotation[2] = glm::vec4(-forward, 0.0f);
        
        model = model * rotation;
    }
    
    // Escala pequena para o boid
    model = glm::scale(model, glm::vec3(0.5f));
    
    return model;
}
