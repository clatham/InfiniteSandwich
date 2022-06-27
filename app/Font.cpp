#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include "Font.h"
#include "glad/glad.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
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
"    FragColor = vec4(1.0,1.0,1.0,texture(ourTexture,texCoord));\n"
"}\n\0";


struct Extent
{
    int x;
    int y;
    int width;
    int height;
    int bearingX;
    int bearingY;
    int advance;
};

struct Font::PrivateImpl
{
    int bitmapWidth;
    int bitmapHeight;
    
    std::map<int,Extent> extents;
    
    unsigned int shaderProgram;
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int texture;
};


Font::Font() :
    m_impl(new PrivateImpl)
{
    if(m_impl)
    {
        m_impl->bitmapWidth = 0;
        m_impl->bitmapHeight = 0;
        
        m_impl->shaderProgram = 0;
        m_impl->vao = 0;
        m_impl->vbo = 0;
        m_impl->ebo = 0;
        m_impl->texture = 0;
    }
}


Font::~Font()
{
    if(m_impl)
    {
        destroy();
        delete m_impl;
    }
}


bool Font::valid() const
{
    if(!m_impl)
        return false;
    
    return m_impl->vao != 0;
}


bool Font::load(const std::string& filename,float lineHeight)
{
    if(!m_impl)
        return false;
    
    
    std::ifstream fontFile(filename,std::ios_base::binary);
    
    if(fontFile.is_open())
    {
        std::streampos fileSize = fontFile.tellg();
        fontFile.seekg(0,std::ios::end);
        fileSize = fontFile.tellg() - fileSize;
        fontFile.seekg(0,std::ios::beg);
        
        
        unsigned char *buffer = new unsigned char[fileSize];
        fontFile.read((char *) buffer,fileSize);
        fontFile.close();
        
        
        stbtt_fontinfo info;
        
        if(::stbtt_InitFont(&info,buffer,::stbtt_GetFontOffsetForIndex(buffer,0)))
        {
            // get the scale
            float scale = ::stbtt_ScaleForPixelHeight(&info,lineHeight);
            
            
            // walk through the printable characters to determine the total width
            m_impl->bitmapWidth = 0;
            m_impl->bitmapHeight = 0;
            
            for(int character = 32;character < 128;++character)
            {
                int x1,y1,x2,y2;
                ::stbtt_GetCodepointBitmapBox(&info,character,scale,scale,&x1,&y1,&x2,&y2);
                
                
                int characterWidth = x2 - x1;
                int characterHeight = y2 - y1;
                
                m_impl->bitmapWidth += characterWidth + 1;
                
                if(characterHeight > m_impl->bitmapHeight)
                    m_impl->bitmapHeight = characterHeight;
            }
            
            
            // allocate the bitmap and set it all to zero
            unsigned char *bitmap = new unsigned char[m_impl->bitmapWidth * m_impl->bitmapHeight];
            memset(bitmap,0,m_impl->bitmapWidth * m_impl->bitmapHeight);
            
            int x = 0;
            
            for(int character = 32;character < 128;++character)
            {
                int advance;
                ::stbtt_GetCodepointHMetrics(&info,character,&advance,NULL);
                
                
                int x1,y1,x2,y2;
                ::stbtt_GetCodepointBitmapBox(&info,character,scale,scale,&x1,&y1,&x2,&y2);
                
                
                int characterWidth = x2 - x1;
                int characterHeight = y2 - y1;
                
                ::stbtt_MakeCodepointBitmap(&info,bitmap + x,characterWidth,characterHeight,m_impl->bitmapWidth,scale,scale,character);
                
                
                Extent extent;
                extent.x = x;
                extent.y = m_impl->bitmapHeight - characterHeight;
                extent.width = characterWidth;
                extent.height = characterHeight;
                extent.bearingX = x1;
                extent.bearingY = y2;
                extent.advance = (int)((float) advance * scale);
                
                m_impl->extents[character] = extent;
                
                
                x += characterWidth + 1;
            }
            
            
            Image image;
            image.load(bitmap,m_impl->bitmapWidth,m_impl->bitmapHeight,8);
            
            
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

            ::glPixelStorei(GL_UNPACK_ALIGNMENT,1);
            ::glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width(),image.height(),
                                         0,GL_RED,GL_UNSIGNED_BYTE,(const char *) image.pixelData());
                                         
            ::glGenerateMipmap(GL_TEXTURE_2D);

            ::glEnable(GL_BLEND);
            ::glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

            // position attribute
            ::glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5 * sizeof(float),(void *) 0);
            ::glEnableVertexAttribArray(0);

            // texture coord attribute
            ::glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5 * sizeof(float),(void *)(3 * sizeof(float)));
            ::glEnableVertexAttribArray(1);
            
            
            delete[] bitmap;
        }        
        else
        {
            std::cerr << "Font::load:  error initializing font" << std::endl;
            return false;
        }

        
        delete[] buffer;
    }
    else
    {
        std::cerr << "Font::load:  error opening font file '" << filename << "'" << std::endl;
        return false;
    }

    
    return true;
}


void Font::destroy()
{
    if(valid())
    {
        ::glDeleteVertexArrays(1,&m_impl->vao);
        ::glDeleteBuffers(1,&m_impl->vbo);
        ::glDeleteBuffers(1,&m_impl->ebo);
        ::glDeleteProgram(m_impl->shaderProgram);
        
        m_impl->bitmapWidth = 0;
        m_impl->bitmapHeight = 0;
        m_impl->shaderProgram = 0;
        m_impl->vao = 0;
        m_impl->vbo = 0;
        m_impl->ebo = 0;
        m_impl->texture = 0;
    }
}


void Font::drawText(const std::string& text,
                    float screenX,float screenY,float screenWidth,float screenHeight)
{
    if(!valid())
        return;
    
    
    GLint viewport[4];
    ::glGetIntegerv(GL_VIEWPORT,viewport);
    
    int viewportWidth = viewport[2];
    int viewportHeight = viewport[3];
    
    float xScale = 2.0f / viewportWidth;
    float yScale = 2.0f / viewportHeight;
    
    
    ::glBindTexture(GL_TEXTURE_2D,m_impl->texture);
    
    ::glUseProgram(m_impl->shaderProgram);
    ::glBindVertexArray(m_impl->vao);

    ::glBindBuffer(GL_ARRAY_BUFFER,m_impl->vbo);
    
    
    float x = screenX;
    
    for(int i = 0;i < text.size();++i)
    {
        Extent extent = m_impl->extents[(unsigned char) text[i]];
/*        std::cout << "x = " << std::setprecision(8) << x
                  << ",  width = " << (float) extent.width * xScale
                  << ",  advance = " << (float) extent.advance * xScale
                  << "\n    extent.x = " << extent.x
                  << ",  extent.width = " << extent.width
                  << ",  extent.advance = " << extent.advance
                  << std::endl;*/
        
        
        float width = (float) extent.width * xScale;
        float height = (float) extent.height * yScale;
        float xOff = (float) extent.bearingX * xScale;
        float yOff = (float) -extent.bearingY * yScale;
        
        float textureX = (float) extent.x / m_impl->bitmapWidth;
        float textureY = (float) extent.y / m_impl->bitmapHeight;
        float textureWidth = (float) extent.width / m_impl->bitmapWidth;
        float textureHeight = (float) extent.height / m_impl->bitmapHeight;
        
        float vertices[] =
        {
            // positions                                                        // texture coords
            x + xOff + width, screenY + yOff + height, 0.0f,  textureX + textureWidth, textureY + textureHeight,
            x + xOff + width, screenY + yOff,          0.0f,  textureX + textureWidth, textureY,
            x + xOff,         screenY + yOff,          0.0f,  textureX, textureY,
            x + xOff,         screenY + yOff + height, 0.0f,  textureX, textureY + textureHeight
        };
        
        ::glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);


        ::glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
//        m_impl->texture.draw(x,screenY,(float) extent.width * xScale,(float) extent.height * yScale,
//                             (float) extent.x / m_impl->bitmapWidth,(float) extent.y / m_impl->bitmapHeight,(float) extent.width / m_impl->bitmapWidth,(float) extent.height / m_impl->bitmapHeight);

        
        x += (float) extent.advance * xScale;
    }

    ::glBindVertexArray(0);
}
