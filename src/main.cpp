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
#include <vector>
#include <random>

#include "shaderClass.hpp"
#include "VAO.hpp"
#include "VBO.hpp"
#include "EBO.hpp"
#include "texture.hpp"
#include "camera.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class vertex{
    public:
        int x;
        int y;
        int z;
};

// definir o tamanho da janela
const unsigned int width = 1200;
const unsigned int height = 1200;

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

void objLoader(std::vector<float> &v, std::vector<int> &e)
{
    std::ifstream inputFile("resource_files/models/elephant.obj");
    if (!inputFile.is_open()) {
        std::cerr << "Erro ao abrir arquivo";
        return;
    }

    std::vector<float> positions; // x,y,z
    std::vector<float> texcoords; // u,v

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
        else if (prefix == "f") {
            // quebrar a linha inteira em tokens
            std::vector<std::string> tokens;
            std::string token;
            std::istringstream fss(line.substr(2)); // ignora "f "
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

                float x=0,y=0,z=0, u=0,vCoord=0, r=0,g=0,b=0;

                if (vid > 0 && (vid-1)*3+2 < (int)positions.size()) {
                    x = positions[(vid-1)*3+0];
                    y = positions[(vid-1)*3+1];
                    z = positions[(vid-1)*3+2];
                }

                if (vtid > 0 && (vtid-1)*2+1 < (int)texcoords.size()) {
                    u = texcoords[(vtid-1)*2+0];
                    vCoord = texcoords[(vtid-1)*2+1];
                }

                v.push_back(x);
                v.push_back(y);
                v.push_back(z);
                v.push_back(r);
                v.push_back(g);
                v.push_back(b);
                v.push_back(u);
                v.push_back(vCoord);

                int newIndex = (v.size()/8) - 1;
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
        v.push_back(0.2f); v.push_back(0.4f); v.push_back(0.2f);
        v.push_back(0.0f); v.push_back(0.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Vértice 2
        v.push_back(p2_top.x); v.push_back(p2_top.y); v.push_back(p2_top.z);
        v.push_back(0.2f); v.push_back(0.4f); v.push_back(0.2f);
        v.push_back(1.0f); v.push_back(0.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Vértice 3
        v.push_back(p1_bot.x); v.push_back(p1_bot.y); v.push_back(p1_bot.z);
        v.push_back(0.3f); v.push_back(0.1f); v.push_back(0.0f);
        v.push_back(0.0f); v.push_back(1.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Segundo triângulo (p2_top, p2_bot, p1_bot)
        // Vértice 4
        v.push_back(p2_top.x); v.push_back(p2_top.y); v.push_back(p2_top.z);
        v.push_back(0.2f); v.push_back(0.4f); v.push_back(0.2f);
        v.push_back(1.0f); v.push_back(0.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Vértice 5
        v.push_back(p2_bot.x); v.push_back(p2_bot.y); v.push_back(p2_bot.z);
        v.push_back(0.3f); v.push_back(0.1f); v.push_back(0.0f);
        v.push_back(1.0f); v.push_back(1.0f);
        v.push_back(faceNormal.x); v.push_back(faceNormal.y); v.push_back(faceNormal.z);
        
        // Vértice 6
        v.push_back(p1_bot.x); v.push_back(p1_bot.y); v.push_back(p1_bot.z);
        v.push_back(0.3f); v.push_back(0.1f); v.push_back(0.0f);
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
            v.push_back(0.5f); v.push_back(0.5f); v.push_back(0.5f);
            v.push_back(0.0f); v.push_back(0.0f);
            v.push_back(normal1.x); v.push_back(normal1.y); v.push_back(normal1.z);
            
            v.push_back(p2.x); v.push_back(p2.y); v.push_back(p2.z);
            v.push_back(0.5f); v.push_back(0.5f); v.push_back(0.5f);
            v.push_back(1.0f); v.push_back(0.0f);
            v.push_back(normal1.x); v.push_back(normal1.y); v.push_back(normal1.z);
            
            v.push_back(p3.x); v.push_back(p3.y); v.push_back(p3.z);
            v.push_back(0.5f); v.push_back(0.5f); v.push_back(0.5f);
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
            v.push_back(0.5f); v.push_back(0.5f); v.push_back(0.5f);
            v.push_back(1.0f); v.push_back(1.0f);
            v.push_back(normal2.x); v.push_back(normal2.y); v.push_back(normal2.z);
            
            v.push_back(p5.x); v.push_back(p5.y); v.push_back(p5.z);
            v.push_back(0.5f); v.push_back(0.5f); v.push_back(0.5f);
            v.push_back(1.0f); v.push_back(0.0f);
            v.push_back(normal2.x); v.push_back(normal2.y); v.push_back(normal2.z);
            
            v.push_back(p6.x); v.push_back(p6.y); v.push_back(p6.z);
            v.push_back(0.5f); v.push_back(0.5f); v.push_back(0.5f);
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
        -0.1f+5.0f, -0.1f, 0.1f,
        -0.1f+5.0f, -0.1f, -0.1f,
        0.1f+5.0f, -0.1f, -0.1f, 
        0.1f+5.0f, -0.1f, 0.1f, 
        -0.1f+5.0f, 0.1f, 0.1f, 
        -0.1f+5.0f, 0.1f, -0.1f, 
        0.1f+5.0f, 0.1f, -0.1f, 
        0.1f+5.0f, 0.1f, 0.1f
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
    // Y é altura, X e Z são o plano horizontal
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
    
    std::vector<float> v_plano;
    std::vector<int> e_plano;
    
    // Criar terreno com subdivisão recursiva
    createFloor(v_plano, e_plano, 0.0f, -5.0f, 0.0f, 200.0f, 0.4f, 80);
    
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
    
    std::cout << "Terreno criado: " << v_plano.size()/8 << " vertices, " << e_plano.size()/3 << " triangulos" << std::endl;






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
    // carrega o openGL com o glad
    gladLoadGL();
    // delimita o espaço pra desenhar
    glViewport(0, 0, width, height);

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

    glm::vec3 planePos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::mat4 planeModel = glm::mat4(1.0f);
    planeModel = glm::translate(planeModel, planePos);


    lightShader.Activate();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
    glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    
    shaderProgram.Activate();
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

    // textura
    std::string texPath = "resource_files/textures/";
    Texture popCat((texPath + "elephant.png").c_str(), GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    popCat.texUnit(shaderProgram, "tex0", 0);

    glEnable(GL_DEPTH_TEST);

    // cria a camera
    Camera camera(width, height, glm::vec3(0.0f, 3.0f, 100.0f));

    while (!glfwWindowShouldClose(window))
    {
        // cor base
        glClearColor(0.6f, 0.7f, 0.70f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        camera.Inputs(window);
        camera.updateMatrix(45.0f, 0.1f, 500.0f);
        //std::cout << "CameraPos:        " << "[" << std::setprecision(2) << camera.Position[0] << " , " << camera.Position[1] << " , " << camera.Position[2] << "]" << "      ";
        //std::cout << "CameraOrientation: " << "[" << std::setprecision(2) << camera.Orientation[0] << " , " << camera.Orientation[1] << " , " << camera.Orientation[2] << "]" << std::endl;
        
        // ativar o programa
        shaderProgram.Activate();
        camera.Matrix(shaderProgram, "camMatrix");
        
        // Passar posição da câmera para o shader
        glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);

        // adicinar a textura
        popCat.Bind();

        // Desenhar o plano (chão) primeiro
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(planeModel));
        VAOPlano.Bind();
        glDrawElements(GL_TRIANGLES, sizeof(indicesPlano) / sizeof(indicesPlano[0]), GL_UNSIGNED_INT, 0);

        
        // Desenhar o cilindro
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(cylinderModel));
        VAO1.Bind();
        // escrever triangulos no buffer e desenhar
        glDrawElements(GL_TRIANGLES, sizeof(indicesCylinder) / sizeof(indicesCylinder[0]), GL_UNSIGNED_INT, 0);
        
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