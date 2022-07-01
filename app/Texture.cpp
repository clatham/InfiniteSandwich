#include <iostream>
#include "glad/glad.h"
#include "Texture.h"


static const char *vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"\n"
"out vec2 texCoord;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos,1.0);\n"
"    texCoord = aTexCoord;\n"
"}\n\0";

static const char *fragmentShaderSource =
"#version 330 core\n"
"in vec2 texCoord;\n"
"\n"
"out vec4 FragColor;\n"
"\n"
"uniform sampler2D ourTexture;\n"
"\n"
"void main()\n"
"{\n"
"    FragColor = texture(ourTexture,texCoord);\n"
"}\n\0";


struct Texture::PrivateImpl
{
    int width;
    int height;
    unsigned int shaderProgram;
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int texture;
};


Texture::Texture() :
    m_impl(new PrivateImpl)
{
    if(m_impl)
    {
        m_impl->width = 0;
        m_impl->height = 0;
        m_impl->shaderProgram = 0;
        m_impl->vao = 0;
        m_impl->vbo = 0;
        m_impl->ebo = 0;
        m_impl->texture = 0;
    }
}


Texture::~Texture()
{
    if(m_impl)
    {
        destroy();
        delete m_impl;
    }
}


bool Texture::create(const Image& image)
{
    if(!m_impl)
        return false;
    
    destroy();
    
    
    m_impl->width = image.width();
    m_impl->height = image.height();
    
    
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
        std::cout << "Texture::create:  error compiling vertex shader:  " << infoLog << std::endl;
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
        std::cout << "Texture::create:  error compiling fragment shader:  " << infoLog << std::endl;
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
        std::cout << "Texture::create:  error linking shader program:  " << infoLog << std::endl;
        return false;
    }
    

    ::glDeleteShader(vertexShader);
    ::glDeleteShader(fragmentShader);




    float vertices[] =
    {
        // positions         // texture coords
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f
    };
    
    unsigned int indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };
    

    ::glGenVertexArrays(1,&m_impl->vao);
    ::glBindVertexArray(m_impl->vao);
    
    ::glGenBuffers(1,&m_impl->vbo);
    ::glBindBuffer(GL_ARRAY_BUFFER,m_impl->vbo);
    ::glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    ::glGenBuffers(1,&m_impl->ebo);
    ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_impl->ebo);
    ::glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);
    
    ::glGenTextures(1,&m_impl->texture);
    ::glBindTexture(GL_TEXTURE_2D,m_impl->texture);
    ::glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    ::glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    ::glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    ::glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    switch(image.bitsPerPixel())
    {
        case 8:
            ::glPixelStorei(GL_UNPACK_ALIGNMENT,1);
            ::glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width(),image.height(),
                                         0,GL_RED,GL_UNSIGNED_BYTE,image.pixelData());
            break;
        
        case 24:
            ::glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width(),image.height(),
                                         0,GL_RGB,GL_UNSIGNED_BYTE,image.pixelData());
            break;
        
        case 32:
            ::glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width(),image.height(),
                                         0,GL_RGBA,GL_UNSIGNED_BYTE,image.pixelData());
            break;
        
        default:
            std::cerr << "Texture::create:  unsupported bit depth '" << image.bitsPerPixel() << "'" << std::endl;
            return false;
    }
                                 
    ::glGenerateMipmap(GL_TEXTURE_2D);
    
    ::glEnable(GL_BLEND);
    ::glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    // position attribute
    ::glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5 * sizeof(float),(void *) 0);
    ::glEnableVertexAttribArray(0);
    
    // texture coord attribute
    ::glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5 * sizeof(float),(void *)(3 * sizeof(float)));
    ::glEnableVertexAttribArray(1);


    return true;
}


void Texture::destroy()
{
    if(!m_impl)
        return;
    
    
    m_impl->width = 0;
    m_impl->height = 0;
    
    if(m_impl->vao)
    {
        ::glDeleteVertexArrays(1,&m_impl->vao);
        m_impl->vao = 0;
    }
    
    if(m_impl->vbo)
    {
        ::glDeleteBuffers(1,&m_impl->vbo);
        m_impl->vbo = 0;
    }
    
    if(m_impl->ebo)
    {
        ::glDeleteBuffers(1,&m_impl->ebo);
        m_impl->ebo = 0;
    }
    
    if(m_impl->texture)
    {
        ::glDeleteTextures(1,&m_impl->texture);
        m_impl->texture = 0;
    }
    
    if(m_impl->shaderProgram)
    {
        ::glDeleteProgram(m_impl->shaderProgram);
        m_impl->shaderProgram = 0;
    }
}


bool Texture::valid() const
{
    if(!m_impl)
        return false;
    
    return m_impl->shaderProgram  &&  m_impl->vao  &&  m_impl->vbo  &&  m_impl->ebo  &&  m_impl->texture;
}


int Texture::width() const
{
    if(!valid())
        return 0;
    
    return m_impl->width;
}


int Texture::height() const
{
    if(!valid())
        return 0;
    
    return m_impl->height;
}


void Texture::draw(float screenX,float screenY,float screenWidth,float screenHeight,
                   float textureX,float textureY,float textureWidth,float textureHeight)
{
    if(!valid())
        return;
    
    
    ::glBindTexture(GL_TEXTURE_2D,m_impl->texture);
    
    ::glUseProgram(m_impl->shaderProgram);
    ::glBindVertexArray(m_impl->vao);


    float vertices[] =
    {
        // positions                                                        // texture coords
        screenX + screenWidth * 0.5f, screenY + screenHeight * 0.5f, 0.0f,  textureX + textureWidth, textureY + textureHeight,
        screenX + screenWidth * 0.5f, screenY - screenHeight * 0.5f, 0.0f,  textureX + textureWidth, textureY,
        screenX - screenWidth * 0.5f, screenY - screenHeight * 0.5f, 0.0f,  textureX, textureY,
        screenX - screenWidth * 0.5f, screenY + screenHeight * 0.5f, 0.0f,  textureX, textureY + textureHeight
    };
    
    ::glBindBuffer(GL_ARRAY_BUFFER,m_impl->vbo);
    ::glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);


    ::glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
    ::glBindVertexArray(0);
}
