#include <iostream>
#include "glad/glad.h"
#include "Rectangle.h"


static const char *vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos,1.0);\n"
"}\n\0";

static const char *fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"\n"
"void main()\n"
"{\n"
"    FragColor = vec4(1.0,1.0,1.0,1.0);\n"
"}\n\0";


struct Rectangle::PrivateImpl
{
    bool created;
    unsigned int shaderProgram;
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
};


Rectangle::Rectangle() :
    m_impl(new PrivateImpl)
{
    if(m_impl)
    {
        m_impl->shaderProgram = 0;
        m_impl->vao = 0;
        m_impl->vbo = 0;
        m_impl->ebo = 0;
    }
}


Rectangle::~Rectangle()
{
    if(m_impl)
    {
        if(m_impl->vao)
            destroy();
        
        delete m_impl;
    }
}


bool Rectangle::create()
{
    if(!m_impl)
        return false;
    
    if(m_impl->vao)
        return false;
    
    
    // create and compile vertex shader
    
    unsigned int vertexShader = ::glCreateShader(GL_VERTEX_SHADER);
    ::glShaderSource(vertexShader,1,&vertexShaderSource,NULL);
    ::glCompileShader(vertexShader);
    
    int success;
    ::glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&success);
    
    if(!success)
    {
        char infoLog[512];
        ::glGetShaderInfoLog(vertexShader,512,NULL,infoLog);
        std::cout << "Rectangle::create:  error compiling vertex shader:  " << infoLog << std::endl;
        return false;
    }
    
    
    // create and compile fragment shader
    
    unsigned int fragmentShader = ::glCreateShader(GL_FRAGMENT_SHADER);
    ::glShaderSource(fragmentShader,1,&fragmentShaderSource,NULL);
    ::glCompileShader(fragmentShader);
    
    ::glGetShaderiv(fragmentShader,GL_COMPILE_STATUS,&success);
    
    if(!success)
    {
        char infoLog[512];
        ::glGetShaderInfoLog(fragmentShader,512,NULL,infoLog);
        std::cout << "Rectangle::create:  error compiling fragment shader:  " << infoLog << std::endl;
        return false;
    }
    
    
    // create and link shader program
    
    m_impl->shaderProgram = ::glCreateProgram();
    ::glAttachShader(m_impl->shaderProgram,vertexShader);
    ::glAttachShader(m_impl->shaderProgram,fragmentShader);
    ::glLinkProgram(m_impl->shaderProgram);
    
    ::glGetProgramiv(m_impl->shaderProgram,GL_LINK_STATUS,&success);
    
    if(!success)
    {
        char infoLog[512];
        ::glGetProgramInfoLog(m_impl->shaderProgram,512,NULL,infoLog);
        std::cout << "Rectangle::create:  error linking shader program:  " << infoLog << std::endl;
        return false;
    }
    

    ::glDeleteShader(vertexShader);
    ::glDeleteShader(fragmentShader);




    float vertices[] =
    {
        // positions
         1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };
    
    unsigned int indices[] =
    {
        0, 1,
        1, 2,
        2, 3,
        3, 0
    };
    

    ::glGenVertexArrays(1,&m_impl->vao);
    ::glBindVertexArray(m_impl->vao);
    
    ::glGenBuffers(1,&m_impl->vbo);
    ::glBindBuffer(GL_ARRAY_BUFFER,m_impl->vbo);
    ::glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    ::glGenBuffers(1,&m_impl->ebo);
    ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_impl->ebo);
    ::glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);
    
    // position attribute
    ::glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3 * sizeof(float),(void *) 0);
    ::glEnableVertexAttribArray(0);
    
    return true;
}


void Rectangle::destroy()
{
    if(!m_impl)
        return;
    
    if(m_impl->vao)
    {
        ::glDeleteVertexArrays(1,&m_impl->vao);
        ::glDeleteBuffers(1,&m_impl->vbo);
        ::glDeleteBuffers(1,&m_impl->ebo);
        ::glDeleteProgram(m_impl->shaderProgram);
        
        m_impl->shaderProgram = 0;
        m_impl->vao = 0;
        m_impl->vbo = 0;
        m_impl->ebo = 0;
    }
}


void Rectangle::draw(float centerX,float centerY,float width,float height)
{
    if(!m_impl)
        return;
    
    if(!m_impl->vao)
        return;
    
    
    ::glUseProgram(m_impl->shaderProgram);
    ::glBindVertexArray(m_impl->vao);


    float vertices[] =
    {
        // positions
        centerX + width * 0.5f, centerY + height * 0.5f, 0.0f,
        centerX + width * 0.5f, centerY - height * 0.5f, 0.0f,
        centerX - width * 0.5f, centerY - height * 0.5f, 0.0f,
        centerX - width * 0.5f, centerY + height * 0.5f, 0.0f,
    };
    
    ::glBindBuffer(GL_ARRAY_BUFFER,m_impl->vbo);
    ::glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    ::glDrawElements(GL_LINES,8,GL_UNSIGNED_INT,0);
    ::glBindVertexArray(0);
}
