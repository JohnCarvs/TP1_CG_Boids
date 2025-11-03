#include "VBO.hpp"

VBO::VBO(GLfloat *vertices, GLsizeiptr size)
{
    glGenBuffers(1, &ID);
    // informa qual VBO será usado
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    // armazena o objeto dos vértices no buffer VBO. GL_STATIC_DRAW especifica que
    // os vértices vão ser modificados uma vez, e usados muitas vezes. Pode ser STREAM, STATIC ou DYNAMIC.
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

void VBO::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}
void VBO::Unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void VBO::Delete()
{
    glDeleteBuffers(1, &ID);
}