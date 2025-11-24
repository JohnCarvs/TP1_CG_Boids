// comando para compilar no MSYS2 (UCRT64) com GLAD:
//  g++ src/helloWorld.cpp src/glad.c -o helloWorld.exe -Iinclude -lglfw3 -lopengl32 -lgdi32

#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <glad/glad.h>  // GLAD
#include <GLFW/glfw3.h> // GLFW
#include <stb/stb_image.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <ctime>
#include <vector>
#include <random>

#include "shaderClass.hpp"
#include "VAO.hpp"
#include "VBO.hpp"
#include "EBO.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "flock.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Árvores globais para os boids evitarem
std::vector<Tree> globalTrees;

class vertex{
    public:
        int x;
        int y;
        int z;
};

// definir o tamanho da janela
unsigned int width = 1200;
unsigned int height = 1200;

void sierpinskiCreate(std::vector<float> &v, std::vector<int> &e, int it, float x, float y, float l)
{
    if (it == 0)
    {
        float h = l * sqrt(3) / 2;

        int startIndex = v.size() / 8;

        v.push_back(x - l / 2);
        v.push_back(y - h / 2);
        v.push_back(0.0f);
        v.push_back(x * tan(3.14 / 3) - y + 0.5);
        v.push_back(x * tan(2 * 3.14 / 3) - y + 0.5);
        v.push_back(x * tan(0) + y + 0.8);
        v.push_back(-0.5);
        v.push_back(0.0); // inferior esquerdo
        v.push_back(x + l / 2);
        v.push_back(y - h / 2);
        v.push_back(0.0f);
        v.push_back(x * tan(3.14 / 3) - y + 0.5);
        v.push_back(x * tan(2 * 3.14 / 3) - y + 0.5);
        v.push_back(x * tan(0) + y + 0.8);
        v.push_back(1.5);
        v.push_back(0.0); // inferior direito
        v.push_back(x);
        v.push_back(y + h / 2);
        v.push_back(0.0f);
        v.push_back(x * tan(3.14 / 3) - y + 0.5);
        v.push_back(x * tan(2 * 3.14 / 3) - y + 0.5);
        v.push_back(x * tan(0) + y + 0.8);
        v.push_back(0.5);
        v.push_back(sqrt(3)); // superior

        e.push_back(startIndex);
        e.push_back(startIndex + 1);
        e.push_back(startIndex + 2);

        return;
    }

    float l2 = l / 2.0f;
    float h2 = l2 * sqrt(3) / 2;

    // inferior esquerdo
    sierpinskiCreate(v, e, it - 1, x - l2 / 2, y - h2 / 2, l2);

    // inferior direito
    sierpinskiCreate(v, e, it - 1, x + l2 / 2, y - h2 / 2, l2);

    // superior
    sierpinskiCreate(v, e, it - 1, x, y + h2 / 2, l2);
}

void objLoader(std::vector<float> &v, std::vector<int> &e, std::string path, float r = 0.3f, float g = 0.2f, float b = 0.1f)
{
    std::ifstream inputFile(path);
    if (!inputFile.is_open()) {
        std::cerr << "Erro ao abrir arquivo";
        return;
    }

    std::vector<float> positions; // x,y,z
    std::vector<float> texcoords; // u,v
    std::vector<float> normals; // x,y,z

    std::string line;
    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;


        if (prefix == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }
        else if (prefix == "vt") {
            float u, vCoord;
            iss >> u >> vCoord;
            texcoords.push_back(u);
            texcoords.push_back(vCoord);
        }
        else if (prefix == "vn") {
            float x, y, z;
            iss >> x >> y >> z;
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        }
        else if (prefix == "f") {
            // quebrar a linha inteira em tokens
            std::vector<std::string> tokens;
            std::string token;
            std::istringstream fss(line.substr(2)); // ignora "f"
            while (fss >> token) tokens.push_back(token);

            auto addVertex = [&](std::string t) {
                int vid = 0, vtid = 0, vnid = 0;
                // detecta se é v, v/vt, v//vn ou v/vt/vn
                if (t.find("//") != std::string::npos) {
                    sscanf(t.c_str(), "%d//%d", &vid, &vnid);
                }
                else if (t.find('/') != std::string::npos) {
                    sscanf(t.c_str(), "%d/%d/%d", &vid, &vtid, &vnid);
                    if (vtid == 0) sscanf(t.c_str(), "%d/%d", &vid, &vtid);
                }
                else {
                    sscanf(t.c_str(), "%d", &vid);
                }

                float x=0,y=0,z=0, u=0,vCoord=0;
                float nx=0, ny=1.0, nz=0; // Normal padrão (aponta para cima)

                if (vid > 0 && (vid-1)*3+2 < (int)positions.size()) {
                    x = positions[(vid-1)*3+0];
                    y = positions[(vid-1)*3+1];
                    z = positions[(vid-1)*3+2];
                }

                if (vtid > 0 && (vtid-1)*2+1 < (int)texcoords.size()) {
                    u = texcoords[(vtid-1)*2+0];
                    vCoord = texcoords[(vtid-1)*2+1];
                }
                
                if (vnid > 0 && (vnid-1)*3+2 < (int)normals.size()) {
                    nx = normals[(vnid-1)*3+0];
                    ny = normals[(vnid-1)*3+1];
                    nz = normals[(vnid-1)*3+2];
                }

                v.push_back(x);
                v.push_back(y);
                v.push_back(z);
                v.push_back(r);
                v.push_back(g);
                v.push_back(b);
                v.push_back(u);
                v.push_back(vCoord);
                v.push_back(nx);
                v.push_back(ny);
                v.push_back(nz);

                int newIndex = (v.size()/11) - 1;
                e.push_back(newIndex);
            };

            // triangulação: se for triângulo → usa 3 vértices
            // se for quad → divide em 2 triângulos (0,1,2) e (0,2,3)
            if (tokens.size() == 3) {
                addVertex(tokens[0]);
                addVertex(tokens[1]);
                addVertex(tokens[2]);
            }
            else if (tokens.size() == 4) {
                addVertex(tokens[0]);
                addVertex(tokens[1]);
                addVertex(tokens[2]);

                addVertex(tokens[0]);
                addVertex(tokens[2]);
                addVertex(tokens[3]);
            }
        }
    }
}

void cylinderCreate(std::vector<float> &v, std::vector<int> &e, float x, float y, float z, float r, float h, int it)
{
    // Para flat shading, cada face precisa ter seus próprios vértices com normal única
    int baseIndex = v.size() / 11;
    
    // Criar faces laterais do cilindro (cada quad = 2 triângulos)
    for(int i=0; i<it; i++){
        float angle1 = 2*M_PI * i / it;
        float angle2 = 2*M_PI * ((i+1) % it) / it;
        
        // Calcular posições dos 4 vértices do quad
        float x1 = sin(angle1);
        float z1 = cos(angle1);
        float x2 = sin(angle2);
        float z2 = cos(angle2);
        
        glm::vec3 p1_top(x + x1*r, y + h/2, z + z1*r);
        glm::vec3 p2_top(x + x2*r, y + h/2, z + z2*r);
        glm::vec3 p1_bot(x + x1*r, y - h/2, z + z1*r);
        glm::vec3 p2_bot(x + x2*r, y - h/2, z + z2*r);
        
        // Calcular normal da face (perpendicular ao quad)
        glm::vec3 edge1 = p2_top - p1_top;
        glm::vec3 edge2 = p1_bot - p1_top;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge2, edge1));
        
        // Primeiro triângulo (p1_top, p2_top, p1_bot)
        // Vértice 1
        v.push_back(p1_top.x); v.push_back(p1_top.y); v.push_back(p1_top.z);
        // v.push_back(0.2f); v.push_back(0.4f); v.push_back(0.2f);
        v.push_back(0.3f); v.push_back(0.15f); v.push_back(0.1f);
        v.push_back(0.0f); v.push_back(0.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Vértice 2
        v.push_back(p2_top.x); v.push_back(p2_top.y); v.push_back(p2_top.z);
        // v.push_back(0.2f); v.push_back(0.4f); v.push_back(0.2f);
        v.push_back(0.3f); v.push_back(0.15f); v.push_back(0.1f);
        v.push_back(1.0f); v.push_back(0.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Vértice 3
        v.push_back(p1_bot.x); v.push_back(p1_bot.y); v.push_back(p1_bot.z);
        v.push_back(0.3f); v.push_back(0.15f); v.push_back(0.1f);
        v.push_back(0.0f); v.push_back(1.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Segundo triângulo (p2_top, p2_bot, p1_bot)
        // Vértice 4
        v.push_back(p2_top.x); v.push_back(p2_top.y); v.push_back(p2_top.z);
        // v.push_back(0.2f); v.push_back(0.4f); v.push_back(0.2f);
        v.push_back(0.3f); v.push_back(0.15f); v.push_back(0.1f);
        v.push_back(1.0f); v.push_back(0.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Vértice 5
        v.push_back(p2_bot.x); v.push_back(p2_bot.y); v.push_back(p2_bot.z);
        v.push_back(0.3f); v.push_back(0.15f); v.push_back(0.1f);
        v.push_back(1.0f); v.push_back(1.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Vértice 6
        v.push_back(p1_bot.x); v.push_back(p1_bot.y); v.push_back(p1_bot.z);
        v.push_back(0.3f); v.push_back(0.15f); v.push_back(0.1f);
        v.push_back(0.0f); v.push_back(1.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Adicionar índices
        int offset = baseIndex + i*6;
        e.push_back(offset + 0);
        e.push_back(offset + 1);
        e.push_back(offset + 2);
        
        e.push_back(offset + 3);
        e.push_back(offset + 4);
        e.push_back(offset + 5);
    }
}

void coneCreate(std::vector<float> &v, std::vector<int> &e, float x, float y, float z, float r, float h, int it)
{
    // Para flat shading, cada face precisa ter seus próprios vértices com normal única
    int baseIndex = v.size() / 11;
    
    // ponto de cima
    glm::vec3 p_top(x, y + h, z);
    
    // Criar faces laterais do cilindro (cada quad = 2 triângulos)
    for(int i=0; i<it; i++){
        float angle1 = 2*M_PI * i / it;
        float angle2 = 2*M_PI * ((i+1) % it) / it;
        
        // Calcular posições dos 4 vértices do quad 
        float x1 = sin(angle1);
        float z1 = cos(angle1);
        float x2 = sin(angle2);
        float z2 = cos(angle2);
        
        
        glm::vec3 p1_bot(x + x1*r, y - h/2, z + z1*r);
        glm::vec3 p2_bot(x + x2*r, y - h/2, z + z2*r);
        
        // Calcular normal da face (perpendicular ao quad)
        glm::vec3 edge1 = p_top - p1_bot;
        glm::vec3 edge2 = p1_bot - p2_bot;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
        


        // Primeiro triângulo (p1_top, p2_top, p1_bot)
        // Vértice 1
        v.push_back(p_top.x); v.push_back(p_top.y); v.push_back(p_top.z);
        v.push_back(0.3f); v.push_back(0.4f); v.push_back(0.1f);
        v.push_back(0.0f); v.push_back(0.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Vértice 2
        v.push_back(p2_bot.x); v.push_back(p2_bot.y); v.push_back(p2_bot.z);
        v.push_back(0.3f); v.push_back(0.4f); v.push_back(0.1f);
        v.push_back(1.0f); v.push_back(0.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Vértice 3
        v.push_back(p1_bot.x); v.push_back(p1_bot.y); v.push_back(p1_bot.z);
        v.push_back(0.3f); v.push_back(0.4f); v.push_back(0.1f);
        v.push_back(0.0f); v.push_back(1.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        
        // Adicionar índices
        int offset = baseIndex + i*3;
        e.push_back(offset + 0);
        e.push_back(offset + 1);
        e.push_back(offset + 2);
    }
}

void createFloor(std::vector<float> &v, std::vector<int> &e, float x, float y, float z, float scale, float max_h, int max_it)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> heightDist(-max_h, max_h);
    
    float min_x = x - scale/2;
    float min_z = z - scale/2;
    float step = scale/(max_it-1);

    // Criar grid temporário de alturas
    std::vector<std::vector<float>> heights(max_it, std::vector<float>(max_it));
    for(int i=0; i<max_it; i++){
        for(int j=0; j<max_it; j++){
            heights[i][j] = y + heightDist(gen);
        }
    }
    
    int baseIndex = v.size() / 11;
    
    // Criar triângulos com vértices únicos (flat shading)
    for(int i=0; i<max_it-1; i++){
        for(int j=0; j<max_it-1; j++){
            float x0 = min_x + i*step;
            float x1 = min_x + (i+1)*step;
            float z0 = min_z + j*step;
            float z1 = min_z + (j+1)*step;
            
            float y00 = heights[i][j];
            float y10 = heights[i+1][j];
            float y01 = heights[i][j+1];
            float y11 = heights[i+1][j+1];
            
            // Primeiro triângulo (0,0 -> 1,0 -> 0,1)
            glm::vec3 p1(x0, y00, z0);
            glm::vec3 p2(x1, y10, z0);
            glm::vec3 p3(x0, y01, z1);
            
            // Invertida para apontar para cima (regra da mão direita)
            glm::vec3 edge1 = p2 - p1;
            glm::vec3 edge2 = p3 - p1;
            glm::vec3 normal1 = glm::normalize(glm::cross(edge2, edge1));
            
            // Adicionar 3 vértices do primeiro triângulo
            v.push_back(p1.x); v.push_back(p1.y); v.push_back(p1.z);
            v.push_back(0.3f); v.push_back(0.4f); v.push_back(0.1f);
            v.push_back(0.0f); v.push_back(0.0f);
            v.push_back(normal1.x); v.push_back(normal1.y); v.push_back(normal1.z);
            
            v.push_back(p2.x); v.push_back(p2.y); v.push_back(p2.z);
            v.push_back(0.3f); v.push_back(0.4f); v.push_back(0.1f);
            v.push_back(1.0f); v.push_back(0.0f);
            v.push_back(normal1.x); v.push_back(normal1.y); v.push_back(normal1.z);
            
            v.push_back(p3.x); v.push_back(p3.y); v.push_back(p3.z);
            v.push_back(0.3f); v.push_back(0.4f); v.push_back(0.1f);
            v.push_back(0.0f); v.push_back(1.0f);
            v.push_back(normal1.x); v.push_back(normal1.y); v.push_back(normal1.z);
            
            // Segundo triângulo (1,1 -> 1,0 -> 0,1)
            glm::vec3 p4(x1, y11, z1);
            glm::vec3 p5(x1, y10, z0);
            glm::vec3 p6(x0, y01, z1);
            
            // Invertida para apontar para cima
            glm::vec3 edge3 = p5 - p4;
            glm::vec3 edge4 = p6 - p4;
            glm::vec3 normal2 = glm::normalize(glm::cross(edge3, edge4));
            
            // Adicionar 3 vértices do segundo triângulo
            v.push_back(p4.x); v.push_back(p4.y); v.push_back(p4.z);
            v.push_back(0.3f); v.push_back(0.4f); v.push_back(0.1f);
            v.push_back(1.0f); v.push_back(1.0f);
            v.push_back(normal2.x); v.push_back(normal2.y); v.push_back(normal2.z);
            
            v.push_back(p5.x); v.push_back(p5.y); v.push_back(p5.z);
            v.push_back(0.3f); v.push_back(0.4f); v.push_back(0.1f);
            v.push_back(1.0f); v.push_back(0.0f);
            v.push_back(normal2.x); v.push_back(normal2.y); v.push_back(normal2.z);
            
            v.push_back(p6.x); v.push_back(p6.y); v.push_back(p6.z);
            v.push_back(0.3f); v.push_back(0.4f); v.push_back(0.1f);
            v.push_back(0.0f); v.push_back(1.0f);
            v.push_back(normal2.x); v.push_back(normal2.y); v.push_back(normal2.z);
            
            // Adicionar índices (6 vértices = 2 triângulos)
            int offset = baseIndex + (i*(max_it-1) + j) * 6;
            e.push_back(offset + 0);
            e.push_back(offset + 1);
            e.push_back(offset + 2);
            
            e.push_back(offset + 3);
            e.push_back(offset + 4);
            e.push_back(offset + 5);
        }
    }
}


int main(int argc, char *argv[])
{
    // inicia a biblioteca de gerenciamento de tela
    glfwInit();
    // especifica a versão e tipo do perfil do GLFW e openGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLfloat lightVertices[] = {
        -1.1f+5.0f, -1.1f, 1.1f,
        -1.1f+5.0f, -1.1f, -1.1f,
        1.1f+5.0f, -1.1f, -1.1f, 
        1.1f+5.0f, -1.1f, 1.1f, 
        -1.1f+5.0f, 1.1f, 1.1f, 
        -1.1f+5.0f, 1.1f, -1.1f, 
        1.1f+5.0f, 1.1f, -1.1f, 
        1.1f+5.0f, 1.1f, 1.1f
    };
    
    GLuint lightIndices[] = {
        0, 1, 2,
        0, 2, 3,
        0, 4, 7,
        0, 7, 3,
        3, 7, 6,
        3, 6, 2,
        2, 6, 5,
        2, 5, 1,
        1, 5, 4,
        1, 4, 0,
        4, 5, 6,
        4, 6, 7
    };

    std::vector<float> v;
    std::vector<int> e;
    
    // cylinderCreate(x, y, z, raio, altura, subdivisões)
    cylinderCreate(v,e, 0.0, 10.0, 0.0, 5, 30, 32);
    GLfloat verticesCylinder[v.size()];
    GLuint indicesCylinder[e.size()];
    for (size_t i = 0; i < v.size(); i++)
    {
        verticesCylinder[i] = v[i];
    }
    for (size_t i = 0; i < e.size(); i++)
    {
        indicesCylinder[i] = e[i];
    }
    
    std::vector<float> v_cone;
    std::vector<int> e_cone;
    
    // cylinderCreate(x, y, z, raio, altura, subdivisões)
    coneCreate(v_cone,e_cone, 0.0, 30.0, 0.0, 10, 30, 32);
    GLfloat vertices_cone[v_cone.size()];
    GLuint indices_cone[e_cone.size()];
    for (size_t i = 0; i < v_cone.size(); i++)
    {
        vertices_cone[i] = v_cone[i];
    }
    for (size_t i = 0; i < e_cone.size(); i++)
    {
        indices_cone[i] = e_cone[i];
    }
    
    std::vector<float> bird_vertices_vec;
    std::vector<int> bird_indices_vec;
    objLoader(bird_vertices_vec, bird_indices_vec, "resource_files/models/bird.obj");

    GLfloat bird_vertices[bird_vertices_vec.size()];
    GLuint bird_indices[bird_indices_vec.size()];
    for (size_t i = 0; i < bird_vertices_vec.size(); i++)
    {
        bird_vertices[i] = bird_vertices_vec[i];
    }
    for (size_t i = 0; i < bird_indices_vec.size(); i++)
    {
        bird_indices[i] = bird_indices_vec[i];
    }
    
    std::vector<float> cadeira_vertices_vec;
    std::vector<int> cadeira_indices_vec;
    objLoader(cadeira_vertices_vec, cadeira_indices_vec, "resource_files/models/cadeira.obj");

    GLfloat cadeira_vertices[cadeira_vertices_vec.size()];
    GLuint cadeira_indices[cadeira_indices_vec.size()];
    for (size_t i = 0; i < cadeira_vertices_vec.size(); i++)
    {
        cadeira_vertices[i] = cadeira_vertices_vec[i];
    }
    for (size_t i = 0; i < cadeira_indices_vec.size(); i++)
    {
        cadeira_indices[i] = cadeira_indices_vec[i];
    }
    
    std::vector<float> fusca_vertices_vec;
    std::vector<int> fusca_indices_vec;
    objLoader(fusca_vertices_vec, fusca_indices_vec, "resource_files/models/fusca.obj", 0.6f, 0.6f, 0.6f);

    GLfloat fusca_vertices[fusca_vertices_vec.size()];
    GLuint fusca_indices[fusca_indices_vec.size()];
    for (size_t i = 0; i < fusca_vertices_vec.size(); i++)
    {
        fusca_vertices[i] = fusca_vertices_vec[i];
    }
    for (size_t i = 0; i < fusca_indices_vec.size(); i++)
    {
        fusca_indices[i] = fusca_indices_vec[i];
    }
    
    std::vector<float> v_plano;
    std::vector<int> e_plano;
    
    // Criar terreno com subdivisão recursiva
    createFloor(v_plano, e_plano, 0.0f, -5.0f, 0.0f, 3000.0f, 10.0f, 50);
    
    GLfloat verticesPlano[v_plano.size()];
    GLuint indicesPlano[e_plano.size()];
    for (size_t i = 0; i < v_plano.size(); i++)
    {
        verticesPlano[i] = v_plano[i];
    }
    for (size_t i = 0; i < e_plano.size(); i++)
    {
        indicesPlano[i] = e_plano[i];
    }
    
    Flock flock;
    // Adicionar boids
    for (int i = 0; i < 50; i++) {
        flock.add();
    }
    
    // Criar árvores espalhadas pelo chão
    std::mt19937 treeGen(static_cast<unsigned int>(time(nullptr)));
    std::uniform_real_distribution<float> treePosX(-500.0f, 500.0f);
    std::uniform_real_distribution<float> treePosZ(-500.0f, 500.0f);
    std::uniform_real_distribution<float> treeRadius(0.5f, 1.0f);
    std::uniform_real_distribution<float> treeHeight(20.0f, 100.0f);
    
    int numTrees = 15;
    for (int i = 0; i < numTrees; i++) {
        Tree tree;
        tree.position = glm::vec3(treePosX(treeGen), -5.0f, treePosZ(treeGen));
        tree.radius = treeRadius(treeGen);
        tree.height = treeHeight(treeGen);
        globalTrees.push_back(tree);
    }
    
    
    
    Tree tree;
    tree.height = 100.0f;
    tree.radius = 0.5f;
    tree.position = glm::vec3(0.0f,0.0f,0.0f);
    globalTrees.push_back(tree);


    // cria uma janela com GLFW nas dimensões e nome escolhidos
    GLFWwindow *window = glfwCreateWindow(width, height, "helloWorld", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // associa a janela especificada ao contexto atual
    glfwMakeContextCurrent(window);
    
    // Definir tamanho mínimo da janela para evitar problemas de aspect ratio
    glfwSetWindowSizeLimits(window, 400, 300, GLFW_DONT_CARE, GLFW_DONT_CARE);
    
    // carrega o openGL com o glad
    gladLoadGL();
    // delimita o espaço pra desenhar
    glViewport(0, 0, width, height);
    
    // Criar a camera antes do callback
    Camera camera(width, height, glm::vec3(0.0f, 3.0f, 100.0f));
    
    // Armazenar ponteiro da câmera no user pointer da janela
    glfwSetWindowUserPointer(window, &camera);
    
    // Callback para redimensionamento da janela
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int w, int h) {
        // Garantir dimensões mínimas
        if (w <= 0) w = 1;
        if (h <= 0) h = 1;
        
        glViewport(0, 0, w, h);
        width = w;
        height = h;
        
        // Recuperar ponteiro da câmera
        Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(win));
        if (cam) {
            cam->width = w;
            cam->height = h;
        }
    });

    Shader shaderProgram("resource_files/shaders/default.vert", "resource_files/shaders/default.frag");

    // VAO, VBO, EBO para o cilindro
    VAO VAO1;
    VAO1.Bind();

    VBO VBO1(verticesCylinder, sizeof(verticesCylinder));
    EBO EBO1(indicesCylinder, sizeof(indicesCylinder));

    // linka o VBO com os atributos dos vertices (coordenadas, cores, texturas e normais)
    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 11 * sizeof(float), (void *)0);
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 11 * sizeof(float), (void *)(3 * sizeof(float)));
    VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 11 * sizeof(float), (void *)(6 * sizeof(float)));
    VAO1.LinkAttrib(VBO1, 3, 3, GL_FLOAT, 11 * sizeof(float), (void *)(8 * sizeof(float)));

    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();
    
    // VAO, VBO, EBO para o cone
    VAO VAO_cone;
    VAO_cone.Bind();

    VBO VBO_cone(vertices_cone, sizeof(vertices_cone));
    EBO EBO_cone(indices_cone, sizeof(indices_cone));

    // linka o VBO com os atributos dos vertices (coordenadas, cores, texturas e normais)
    VAO_cone.LinkAttrib(VBO_cone, 0, 3, GL_FLOAT, 11 * sizeof(float), (void *)0);
    VAO_cone.LinkAttrib(VBO_cone, 1, 3, GL_FLOAT, 11 * sizeof(float), (void *)(3 * sizeof(float)));
    VAO_cone.LinkAttrib(VBO_cone, 2, 2, GL_FLOAT, 11 * sizeof(float), (void *)(6 * sizeof(float)));
    VAO_cone.LinkAttrib(VBO_cone, 3, 3, GL_FLOAT, 11 * sizeof(float), (void *)(8 * sizeof(float)));

    VAO_cone.Unbind();
    VBO_cone.Unbind();
    EBO_cone.Unbind();
    
    // VAO, VBO, EBO para o pássaro
    VAO VAO_bird;
    VAO_bird.Bind();

    VBO VBO_bird(bird_vertices, sizeof(bird_vertices));
    EBO EBO_bird(bird_indices, sizeof(bird_indices));

    // linka o VBO com os atributos dos vertices (coordenadas, cores, texturas e normais)
    VAO_bird.LinkAttrib(VBO_bird, 0, 3, GL_FLOAT, 11 * sizeof(float), (void *)0);
    VAO_bird.LinkAttrib(VBO_bird, 1, 3, GL_FLOAT, 11 * sizeof(float), (void *)(3 * sizeof(float)));
    VAO_bird.LinkAttrib(VBO_bird, 2, 2, GL_FLOAT, 11 * sizeof(float), (void *)(6 * sizeof(float)));
    VAO_bird.LinkAttrib(VBO_bird, 3, 3, GL_FLOAT, 11 * sizeof(float), (void *)(8 * sizeof(float)));

    VAO_bird.Unbind();
    VBO_bird.Unbind();
    EBO_bird.Unbind();

    // VAO, VBO, EBO para a cadeira
    VAO VAO_cadeira;
    VAO_cadeira.Bind();

    VBO VBO_cadeira(cadeira_vertices, sizeof(cadeira_vertices));
    EBO EBO_cadeira(cadeira_indices, sizeof(cadeira_indices));

    VAO_cadeira.LinkAttrib(VBO_cadeira, 0, 3, GL_FLOAT, 11 * sizeof(float), (void *)0);
    VAO_cadeira.LinkAttrib(VBO_cadeira, 1, 3, GL_FLOAT, 11 * sizeof(float), (void *)(3 * sizeof(float)));
    VAO_cadeira.LinkAttrib(VBO_cadeira, 2, 2, GL_FLOAT, 11 * sizeof(float), (void *)(6 * sizeof(float)));
    VAO_cadeira.LinkAttrib(VBO_cadeira, 3, 3, GL_FLOAT, 11 * sizeof(float), (void *)(8 * sizeof(float)));

    VAO_cadeira.Unbind();
    VBO_cadeira.Unbind();
    EBO_cadeira.Unbind();

    // VAO, VBO, EBO para o fusca
    VAO VAO_fusca;
    VAO_fusca.Bind();

    VBO VBO_fusca(fusca_vertices, sizeof(fusca_vertices));
    EBO EBO_fusca(fusca_indices, sizeof(fusca_indices));

    // linka o VBO com os atributos dos vertices (coordenadas, cores, texturas e normais)
    VAO_fusca.LinkAttrib(VBO_fusca, 0, 3, GL_FLOAT, 11 * sizeof(float), (void *)0);
    VAO_fusca.LinkAttrib(VBO_fusca, 1, 3, GL_FLOAT, 11 * sizeof(float), (void *)(3 * sizeof(float)));
    VAO_fusca.LinkAttrib(VBO_fusca, 2, 2, GL_FLOAT, 11 * sizeof(float), (void *)(6 * sizeof(float)));
    VAO_fusca.LinkAttrib(VBO_fusca, 3, 3, GL_FLOAT, 11 * sizeof(float), (void *)(8 * sizeof(float)));

    VAO_fusca.Unbind();
    VBO_fusca.Unbind();
    EBO_fusca.Unbind();

    // VAO, VBO, EBO para o plano (chão)
    VAO VAOPlano;
    VAOPlano.Bind();
    VBO VBOPlano(verticesPlano, sizeof(verticesPlano));
    EBO EBOPlano(indicesPlano, sizeof(indicesPlano));

    VAOPlano.LinkAttrib(VBOPlano, 0, 3, GL_FLOAT, 11 * sizeof(float), (void *)0);
    VAOPlano.LinkAttrib(VBOPlano, 1, 3, GL_FLOAT, 11 * sizeof(float), (void *)(3 * sizeof(float)));
    VAOPlano.LinkAttrib(VBOPlano, 2, 2, GL_FLOAT, 11 * sizeof(float), (void *)(6 * sizeof(float)));
    VAOPlano.LinkAttrib(VBOPlano, 3, 3, GL_FLOAT, 11 * sizeof(float), (void *)(8 * sizeof(float)));

    VAOPlano.Unbind();
    VBOPlano.Unbind();
    EBOPlano.Unbind();


    // VAO, VBO, EBO para o cubo de luz
    Shader lightShader("resource_files/shaders/light.vert", "resource_files/shaders/light.frag");

    VAO lightVAO;
    lightVAO.Bind();

    VBO lightVBO(lightVertices, sizeof(lightVertices));
    EBO lightEBO(lightIndices, sizeof(lightIndices));

    lightVAO.LinkAttrib(lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

    lightVAO.Unbind();
    lightVBO.Unbind();
    lightEBO.Unbind();

    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(10.0f, 10.0f, 10.0f);
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::translate(lightModel, lightPos);
    lightModel = glm::scale(lightModel, glm::vec3(0.5f)); // Cubo de luz menor

    glm::vec3 cylinderPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::mat4 cylinderModel = glm::mat4(1.0f);
    cylinderModel = glm::translate(cylinderModel, cylinderPos);
   
    glm::vec3 cone_Pos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::mat4 cone_model = glm::mat4(1.0f);
    cone_model = glm::translate(cone_model, cone_Pos);
    
    glm::vec3 birdPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::mat4 birdModel = glm::mat4(1.0f);
    birdModel = glm::translate(birdModel, birdPos);

    glm::vec3 fuscaPos = glm::vec3(125.0f, 15.0f, 20.0f);
    glm::mat4 fuscaModel = glm::mat4(1.0f);
    fuscaModel = glm::translate(fuscaModel, fuscaPos);
    fuscaModel = glm::scale(fuscaModel, glm::vec3(0.5f));

    glm::vec3 planePos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::mat4 planeModel = glm::mat4(1.0f);
    planeModel = glm::translate(planeModel, planePos);


    lightShader.Activate();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
    glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    
    shaderProgram.Activate();
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

    // Configurar fog
    bool fogEnabled = false;
    glm::vec3 fogColor = glm::vec3(0.6f, 0.7f, 0.70f); // Mesma cor do fundo
    //glm::vec3 fogColor = glm::vec3(0.6f, 0.0f, 0.70f); 
    float fogStart = 50.0f;
    float fogEnd = 600.0f;
    
    glUniform1i(glGetUniformLocation(shaderProgram.ID, "fogEnabled"), fogEnabled);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "fogColor"), fogColor.x, fogColor.y, fogColor.z);
    glUniform1f(glGetUniformLocation(shaderProgram.ID, "fogStart"), fogStart);
    glUniform1f(glGetUniformLocation(shaderProgram.ID, "fogEnd"), fogEnd);

    // textura
    std::string texPath = "resource_files/textures/";
    Texture popCat((texPath + "elephant.png").c_str(), GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    popCat.texUnit(shaderProgram, "tex0", 0);

    glEnable(GL_DEPTH_TEST);

    // Variáveis para deltaTime
    float lastTime = glfwGetTime();
    float deltaTime = 0.0f;
    
    // Variáveis de controle
    bool isPaused = false;
    bool useChairModel = false;

    while (!glfwWindowShouldClose(window))
    {
        // Calcular deltaTime
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // cor base
        glClearColor(0.6f, 0.7f, 0.70f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        camera.Inputs(window, deltaTime);
        
        // Toggle fog com tecla F
        static bool fKeyWasPressed = false;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            if (!fKeyWasPressed) {
                fogEnabled = !fogEnabled;
                shaderProgram.Activate();
                glUniform1i(glGetUniformLocation(shaderProgram.ID, "fogEnabled"), fogEnabled);
                std::cout << "Fog: " << (fogEnabled ? "ligado" : "desligado") << std::endl;
                fKeyWasPressed = true;
            }
        } else {
            fKeyWasPressed = false;
        }
        
        // Toggle pausa com tecla P
        static bool pKeyWasPressed = false;
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            if (!pKeyWasPressed) {
                isPaused = !isPaused;
                std::cout << (isPaused ? "pausado" : "retomado") << std::endl;
                pKeyWasPressed = true;
            }
        } else {
            pKeyWasPressed = false;
        }
        
        // Toggle modelo alternativo com tecla M
        static bool mKeyWasPressed = false;
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            if (!mKeyWasPressed) {
                useChairModel = !useChairModel;
                std::cout << "Modelo: " << (useChairModel ? "cadeira" : "passaro") << std::endl;
                mKeyWasPressed = true;
            }
        } else {
            mKeyWasPressed = false;
        }
        
        // Controlar velocidade do líder (boid objetivo) - alterar módulo
        if (!flock.getBoids().empty() && flock.getBoids()[0].isObjective) {
            Boid& leader = flock.getBoids()[0];
            float speedChange = 20.0f * deltaTime;
            
            // Aumentar ou diminuir o módulo da velocidade
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                float currentSpeed = glm::length(leader.velocity);
                if (currentSpeed > 0.1f) {
                    leader.velocity = glm::normalize(leader.velocity) * (currentSpeed + speedChange);
                } else {
                    // Se velocidade é zero, dar uma velocidade inicial
                    leader.velocity = glm::vec3(0.0f, 0.0f, speedChange);
                }
                std::cout << "Velocidade lider: " << glm::length(leader.velocity) << std::endl;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                float currentSpeed = glm::length(leader.velocity);
                float newSpeed = glm::max(0.0f, currentSpeed - speedChange);
                if (currentSpeed > 0.1f) {
                    leader.velocity = glm::normalize(leader.velocity) * newSpeed;
                }
                std::cout << "Velocidade lider: " << glm::length(leader.velocity) << std::endl;
            }
            
            // Limitar velocidade máxima do líder
            float maxLeaderSpeed = 100.0f;
            if (glm::length(leader.velocity) > maxLeaderSpeed) {
                leader.velocity = glm::normalize(leader.velocity) * maxLeaderSpeed;
            }
        }
        
        // Passar os boids para a camera
        camera.updateMatrix(90.0f, 0.1f, 2000.0f, flock);
        
        //std::cout << "CameraPos:        " << "[" << std::setprecision(2) << camera.Position[0] << " , " << camera.Position[1] << " , " << camera.Position[2] << "]" << "      ";
        //std::cout << "CameraOrientation: " << "[" << std::setprecision(2) << camera.Orientation[0] << " , " << camera.Orientation[1] << " , " << camera.Orientation[2] << "]" << std::endl;
        
        // ativar o programa
        shaderProgram.Activate();
        camera.Matrix(shaderProgram, "camMatrix");
        
        // Passar posição da câmera para o shader
        glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        
        // Inicializar wingPhase com valor padrão (0.0) para objetos sem animação
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "wingPhase"), 0.0f);

        // adicinar a textura
        popCat.Bind();

        // Desenhar o plano (chão) primeiro
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(planeModel));
        VAOPlano.Bind();
        glDrawElements(GL_TRIANGLES, sizeof(indicesPlano) / sizeof(indicesPlano[0]), GL_UNSIGNED_INT, 0);
        
        // Atualizar e desenhar os pássaros (boids)
        flock.inputs(window);
        if (!isPaused) {
            flock.update(deltaTime, 600.0f, 200.0f, 600.0f);  // limites X, Y, Z
        }
        
        // Escolher modelo baseado no modo
        if (useChairModel) {
            VAO_cadeira.Bind();
        } else {
            VAO_bird.Bind();
        }
        for (const auto& boid : flock.getBoids()) {
            glm::mat4 model = boid.getModelMatrix();
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
            
            // Cadeira não tem animação de asas
            if (useChairModel) {
                glUniform1f(glGetUniformLocation(shaderProgram.ID, "wingPhase"), 0.0f);
                glDrawElements(GL_TRIANGLES, sizeof(cadeira_indices) / sizeof(cadeira_indices[0]), GL_UNSIGNED_INT, 0);
            } else {
                glUniform1f(glGetUniformLocation(shaderProgram.ID, "wingPhase"), boid.wingPhase);
                glDrawElements(GL_TRIANGLES, sizeof(bird_indices) / sizeof(bird_indices[0]), GL_UNSIGNED_INT, 0);
            }
        }

        // Resetar wingPhase para objetos estáticos (cilindro, cone, etc)
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "wingPhase"), 0.0f);

        // Desenhar todas as árvores
        for (const auto& tree : globalTrees) {
            // Desenhar o tronco (cilindro)
            glm::mat4 trunkModel = glm::mat4(1.0f);
            trunkModel = glm::translate(trunkModel, tree.position);
            trunkModel = glm::translate(trunkModel, glm::vec3(0.0f, 0.0f, 0.0f));
            trunkModel = glm::scale(trunkModel, glm::vec3(tree.radius + tree.height/80, tree.height /10, tree.radius + tree.height/80));
            
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(trunkModel));
            VAO1.Bind();
            glDrawElements(GL_TRIANGLES, sizeof(indicesCylinder) / sizeof(indicesCylinder[0]), GL_UNSIGNED_INT, 0);
            
            // Desenhar a copa (cone)
            glm::mat4 foliageModel = glm::mat4(1.0f);
            foliageModel = glm::translate(foliageModel, tree.position);
            foliageModel = glm::translate(foliageModel, glm::vec3(0.0f, 0.0f, 0.0f));
            foliageModel = glm::scale(foliageModel, glm::vec3(tree.radius + tree.height/80, tree.height /10, tree.radius + tree.height/80));
            
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(foliageModel));
            VAO_cone.Bind();
            glDrawElements(GL_TRIANGLES, sizeof(indices_cone) / sizeof(indices_cone[0]), GL_UNSIGNED_INT, 0);
        }
        
        // Desenhar o fusca
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(fuscaModel));
        VAO_fusca.Bind();
        glDrawElements(GL_TRIANGLES, sizeof(fusca_indices) / sizeof(fusca_indices[0]), GL_UNSIGNED_INT, 0);
        
        // Desenhar a luz
        lightShader.Activate();
        camera.Matrix(lightShader, "camMatrix");
        lightVAO.Bind();
        glDrawElements(GL_TRIANGLES, sizeof(lightIndices) / sizeof(lightIndices[0]), GL_UNSIGNED_INT, 0);


        glfwSwapBuffers(window);

        // processar todos os eventos da tela
        glfwPollEvents();
    }

    // deletar tudo
    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    VAOPlano.Delete();
    VBOPlano.Delete();
    EBOPlano.Delete();
    shaderProgram.Delete();
    glfwDestroyWindow(window);
    popCat.Delete();
    glfwTerminate();
    return 0;
}