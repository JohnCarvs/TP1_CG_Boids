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
    // função geradora de cilindro com centro no (x,y,z), raio r e altura h
    // topo
    for(int i=0; i<it; i++){
        v.push_back(x + sin(2*M_PI * i / it)*r);    // x
        v.push_back(z + h/2);                       // z
        v.push_back(y + cos(2*M_PI * i / it)*r);    // y
        v.push_back(0.5);                           // r
        v.push_back(0.1);                           // g
        v.push_back(0.8);                           // b
        v.push_back(0.0);                           // u
        v.push_back(0.0);                           // v
    }
    // base
    for(int i=0; i<it; i++){
        v.push_back(x + sin(2*M_PI * i / it)*r);    // x
        v.push_back(z - h/2);                       // z  
        v.push_back(y + cos(2*M_PI * i / it)*r);    // y                      
        v.push_back(0.5);                           // r
        v.push_back(0.1);                           // g
        v.push_back(0.8);                           // b
        v.push_back(0.0);                           // u
        v.push_back(0.0);                           // v
    }
    // faces laterais
    for(int i=0; i<it; i++){
        int next = (i+1)%it;
        
        int top1 = i;
        int top2 = next;
        int bottom1 = i+it;
        int bottom2 = next+it;

        e.push_back(top1);
        e.push_back(top2);
        e.push_back(bottom1);

        e.push_back(top2);
        e.push_back(bottom1);
        e.push_back(bottom2);
    }
}

void createFloor(std::vector<float> &v, std::vector<int> &e, float x, float y, float z, float scale, float max_h, int max_it)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> heightDist(-max_h, max_h);
    
    float min_x = x - scale/2;
    float min_z = z - scale/2;
    float max_x = x + scale/2;
    float max_z = z + scale/2;

    float step = scale/(max_it-1);

    for(int i=0; i<max_it; i++){
        for(int j=0; j<max_it; j++){
            v.push_back(min_x + i*step);
            v.push_back(y + heightDist(gen));
            v.push_back(min_z + j*step);
            v.push_back(0.0);
            v.push_back(0.0);
            v.push_back(0.0);
            v.push_back(0.0);
            v.push_back(0.0);

        }
    }
    
    for(int i=0; i<max_it-1; i++){
        for(int j=0; j<max_it-1; j++){
            e.push_back(i*max_it + j);
            e.push_back(i*max_it + j+1);
            e.push_back(i*max_it + j+max_it);
            
            e.push_back(i*max_it + j+max_it+1);
            e.push_back(i*max_it + j+1);
            e.push_back(i*max_it + j+max_it);
            
        }
    }
    

}

int main(int argc, char *argv[])
{
    int arg0 = std::atoi(argv[1]);

    // inicia a biblioteca de gerenciamento de tela
    glfwInit();
    // especifica a versão e tipo do perfil do GLFW e openGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    std::vector<float> v;
    std::vector<int> e;

    cylinderCreate(v,e, 0.0, 5.0, 0.0, 5, 10, 32);
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
    createFloor(v_plano, e_plano, 0.0f, -5.0f, 0.0f, 40.0f, 0.4f, 20);
    
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

    // linka o VBO com os atributos dos vertices (coordenadas e cores)
    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void *)0);
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void *)(6 * sizeof(float)));

    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();

    // VAO, VBO, EBO para o plano (chão)
    VAO VAOPlano;
    VAOPlano.Bind();

    VBO VBOPlano(verticesPlano, sizeof(verticesPlano));
    EBO EBOPlano(indicesPlano, sizeof(indicesPlano));

    VAOPlano.LinkAttrib(VBOPlano, 0, 3, GL_FLOAT, 8 * sizeof(float), (void *)0);
    VAOPlano.LinkAttrib(VBOPlano, 1, 3, GL_FLOAT, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    VAOPlano.LinkAttrib(VBOPlano, 2, 2, GL_FLOAT, 8 * sizeof(float), (void *)(6 * sizeof(float)));

    VAOPlano.Unbind();
    VBOPlano.Unbind();
    EBOPlano.Unbind();

    // textura
    std::string texPath = "resource_files/textures/";
    Texture popCat((texPath + "elephant.png").c_str(), GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    popCat.texUnit(shaderProgram, "tex0", 0);

    glEnable(GL_DEPTH_TEST);

    // cria a camera
    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

    while (!glfwWindowShouldClose(window))
    {
        // cor base
        glClearColor(0.08f, 0.12f, 0.30f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ativar o programa
        shaderProgram.Activate();

        camera.Inputs(window);
        camera.Matrix(45.0f, 0.1f, 100.0f, shaderProgram, "camMatrix");
        //std::cout << "CameraPos:        " << "[" << std::setprecision(2) << camera.Position[0] << " , " << camera.Position[1] << " , " << camera.Position[2] << "]" << "      ";
        //std::cout << "CameraOrientation: " << "[" << std::setprecision(2) << camera.Orientation[0] << " , " << camera.Orientation[1] << " , " << camera.Orientation[2] << "]" << std::endl;

        // adicinar a textura
        popCat.Bind();

        // Desenhar o plano (chão) primeiro
        VAOPlano.Bind();
        glDrawElements(GL_TRIANGLES, sizeof(indicesPlano) / sizeof(indicesPlano[0]), GL_UNSIGNED_INT, 0);

        // Desenhar o cilindro
        VAO1.Bind();
        // escrever triangulos no buffer e desenhar
        glDrawElements(GL_TRIANGLES, sizeof(indicesCylinder) / sizeof(indicesCylinder[0]), GL_UNSIGNED_INT, 0);
        
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