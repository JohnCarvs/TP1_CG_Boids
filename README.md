
# Boids - Trabalho Prático 1

Implementação do algoritmo de Boids (Craig Reynolds) com renderização 3D em OpenGL, incluindo sistema de câmera, iluminação Phong, oclusão de ambiente e animações procedurais.

## Controles

### Movimentação da Câmera (Modo Livre)
- W/A/S/D - Movimentação
- Espaço - Subir
- Ctrl - Descer
- Shift - 2x velocidade
- Mouse (Botão Esquerdo) - Olhar ao redor
- I - Mostrar posição e orientação da câmera no console

### Modos de Câmera
- 1 - Modo fixo: câmera fixa no centro do mundo olhando para o centro do bando
- 2 - Modo Seguir: câmera atrás do bando a uma distância fixa, seguindo o movimento
- 3 - Modo Perpendicular: câmera perpendicular ao vetor velocidade do bando, paralela ao chão
- 4 - Voltar ao modo de navegação livre

### Controle de Boids
- UP/DOWN - Aumentar/diminuir velocidade do boid líder
- Numpad + - Adicionar um boid
- Numpad * - Adicionar um boid no centro do bando
- Numpad / - Spawn de boid com velocidade inicial para colidir com árvore central
- Numpad - - Remover boids aleatórios
- V - Toggle para sempre perceber o líder (ignora limite de percepção)

### Alternância de Modelos e Efeitos
- M - Alternar entre modelo de pássaro e cadeira para os boids
- P - Pausar/retomar simulação
- F - Ligar/Desligar fog

## Compilação / Execução

```bash
make
./bin/main.exe
```

## Características Implementadas

### Algoritmo de Boids
Implementação do algoritmo de Reynolds com 4 comportamentos principais:
- Separação: Evita colisões mantendo distância mínima entre boids
- Alinhamento: Ajusta velocidade para seguir a direção média do grupo
- Coesão: Move-se em direção ao centro de massa do grupo percebido
- Objetivo: Segue o boid líder com comportamento diferenciado

O primeiro boid criado é sempre designado como líder, com velocidade máxima maior (100.0) comparado aos seguidores (35.0). Os boids possuem campo de percepção limitado por distância, mas podem ser configurados para sempre perceber o líder independente da distância.

### Sistema de Iluminação
- Modelo Phong completo implementado no fragment shader (default.frag)
- Componentes ambiente, difusa e especular
- Luz pontual posicionável no espaço 3D
- Oclusão de Ambiente (AO) procedural:
  - Fator baseado em altura: objetos próximos ao chão (y < -10) ficam até 60% mais escuros
  - Fator baseado em normal: superfícies voltadas para baixo ficam até 40% mais escuras
  - Cálculo: ao = mix(0.4, 1.0, heightFactor) * mix(0.6, 1.0, normalAO)
  - Aplicado ao componente ambiente: ambient = 0.70 * ao

### Renderização e Geometria
- Flat Shading: Normais calculadas por face para aparência facetada
- VBO/VAO/EBO: Uso eficiente de buffers OpenGL para todos os objetos
- Geradores procedurais:
  - generateCone(): Gera vértices e índices para cones (árvores)
  - generateCylinder(): Gera vértices e índices para cilindros (troncos)
  - Plano procedural com variação de altura para terreno
- Carregamento de modelos OBJ:
  - Função objLoader() customizada com parâmetros RGB
  - Suporte para modelos: bird.obj, cadeira.obj, fusca.obj
  - Alternância em tempo real entre modelos de boid

### Animação
- Bater de asas: Animação procedural usando variável wingPhase
- wingPhase incrementado por boid no método update()
- Vertex shader (default.vert) modifica posição Y dos vértices baseado em:
  - Seno do wingPhase para movimento oscilatório
- Animação aplicada apenas quando wingPhase > 0

### Sistema de Câmera
Quatro modos de visualização com transição suave (smoothness = 0.15):

1. Modo Fixo: Posição fixa (0, 60, 0), sempre olha para centro do bando
2. Modo Seguir: Posicionada atrás do bando, segue movimento mantendo distância
3. Modo Perpendicular: Lateral ao bando, perpendicular à direção de movimento
4. Modo Livre: Navegação first-person completa (WASD + mouse)

Todos os modos preservam posição anterior para retorno suave ao modo livre.

### Gestão de Janela
- Reshape dinâmico: Callback framebuffer_size_callback() ajusta viewport e aspect ratio
- Matriz de projeção recalculada automaticamente ao redimensionar
- FOV, near plane e far plane mantidos consistentes

### Obstáculos
- Sistema de desvio de obstáculos cilíndricos (árvores)
- 15 árvores posicionadas aleatoriamente + 1 árvore central alta no spawn
- Boids aplicam força de repulsão ao detectar obstáculos próximos
- Árvores compostas por cone (copa) + cilindro (tronco)

### Fog
- Fog linear implementado no fragment shader
- Parâmetros: fogStart = 50.0, fogEnd = 600.0
- Cor do fog ajustável, mixado com cor final do fragmento
- Toggle em tempo real com tecla F

### Extras
- Modelo de carro (fusca) estático como objeto de cena decorativo
- Console log para todos os comandos e mudanças de estado
- Pausar simulação mantendo controles de câmera e manipulação de boids ativos
- Mudar o raio de percepção dos boids para o líder
- Geração do cenário procedural

## Dependências

- OpenGL 3.3+
- GLFW3
- GLAD
- GLM (matemática vetorial e matricial)
- STB Image (carregamento de texturas)

## Detalhes de Implementação

### Pipeline de Renderização
1. Clear buffers (color + depth)
2. Processar inputs (câmera, boids, toggles)
3. Atualizar boids (se não pausado)
4. Atualizar matriz de câmera
5. Para cada objeto renderizável:
   - Ativar shader program
   - Enviar uniforms (model matrix, wingPhase, camPos, etc)
   - Bind VAO
   - Draw call (glDrawElements)

### Uniformes do Shader
- model: Matriz de transformação do objeto
- camMatrix: Matriz view-projection combinada
- camPos: Posição da câmera (para especular)
- wingPhase: Fase da animação de asas (0.0 = sem animação)
- fogEnabled: Boolean para ativar/desativar fog
- fogStart, fogEnd, fogColor: Parâmetros do fog
