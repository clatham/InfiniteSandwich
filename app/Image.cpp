#include <iostream>
#include <memory>
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct Image::PrivateImpl
{
    unsigned char *data;
    bool useStdDelete;
    int width;
    int height;
    int bitsPerPixel;
};


Image::Image() :
    m_impl(new PrivateImpl)
{
    if(m_impl)
    {
        m_impl->data = NULL;
        m_impl->useStdDelete = true;
        m_impl->width = 0;
        m_impl->height = 0;
        m_impl->bitsPerPixel = 0;
    }
}


Image::Image(const Image& rhs) :
    m_impl(new PrivateImpl)
{
    if(m_impl)
    {
        m_impl->data = NULL;
        m_impl->useStdDelete = true;
        m_impl->width = 0;
        m_impl->height = 0;
        m_impl->bitsPerPixel = 0;
        
        if(rhs.m_impl)
        {
            load(rhs.m_impl->data,rhs.m_impl->width,rhs.m_impl->height,rhs.m_impl->bitsPerPixel,false);
        }
    }
}


Image::Image(Image&& rhs) :
    m_impl(rhs.m_impl)
{
    rhs.m_impl = NULL;
}


Image::~Image()
{
    if(m_impl)
    {
        if(m_impl->data)
            destroy();
        
        delete m_impl;
    }
}


bool Image::load(const std::string& filename)
{
    if(!m_impl)
        return false;
    
    if(m_impl->data)
        destroy();
    
    
    ::stbi_set_flip_vertically_on_load(true);
    
    
    int width,height,bytes;
    m_impl->data = ::stbi_load(filename.c_str(),&width,&height,&bytes,0);
    
    if(!m_impl->data)
    {
        std::cerr << "Image::load:  error loading file:  " << stbi_failure_reason() << std::endl;
        return false;
    }
    
    m_impl->useStdDelete = false;
    m_impl->width = width;
    m_impl->height = height;
    m_impl->bitsPerPixel = 8 * bytes;
    
    return true;
}


bool Image::load(const unsigned char *data,int length)
{
    if(!m_impl)
        return false;
    
    if(m_impl->data)
        destroy();
    
    
    ::stbi_set_flip_vertically_on_load(true);
    
    
    int width,height,bytes;
    m_impl->data = ::stbi_load_from_memory(data,length,&width,&height,&bytes,0);
    
    if(!m_impl->data)
    {
        std::cerr << "Image::load:  error loading file:  " << stbi_failure_reason() << std::endl;
        return false;
    }
    
    m_impl->useStdDelete = false;
    m_impl->width = width;
    m_impl->height = height;
    m_impl->bitsPerPixel = 8 * bytes;
    
    return true;
}


bool Image::load(const unsigned char *data,int width,int height,int bitsPerPixel,bool invert)
{
    if(!m_impl)
        return false;
    
    if(m_impl->data)
        destroy();
    
    
    m_impl->data = new unsigned char[width * height * (bitsPerPixel / 8)];
    int stride = width * (bitsPerPixel / 8);
    
    if(invert)
    {
        // copy line-by-line, flipping the image vertically
        for(int y = 0;y < height;++y)
        {
            memcpy(m_impl->data + y * stride,data + (height - y - 1) * stride,stride);
        }
    }
    else
    {
        memcpy(m_impl->data,data,height * stride);
    }
    
    m_impl->useStdDelete = true;
    m_impl->width = width;
    m_impl->height = height;
    m_impl->bitsPerPixel = bitsPerPixel;
    
    return true;
}


void Image::destroy()
{
    if(!m_impl)
        return;
    
    if(!m_impl->data)
        return;
    
    
    if(m_impl->useStdDelete)
        delete[] m_impl->data;
    else
        ::stbi_image_free(m_impl->data);
    
    m_impl->data = NULL;
    m_impl->width = 0;
    m_impl->height = 0;
    m_impl->bitsPerPixel = 0;
}


int Image::width() const
{
    if(!m_impl)
        return 0;
    
    
    return m_impl->width;
}


int Image::height() const
{
    if(!m_impl)
        return 0;
    
    
    return m_impl->height;
}


int Image::bitsPerPixel() const
{
    if(!m_impl)
        return 0;
    
    
    return m_impl->bitsPerPixel;
}


const void *Image::pixelData() const
{
    if(!m_impl)
        return NULL;
    
    
    return m_impl->data;
}


Image& Image::operator=(const Image& rhs)
{
    Image(rhs).swap(*this);
    return *this;
}


Image& Image::operator=(Image&& rhs)
{
    swap(rhs);
    return *this;
}


void Image::swap(Image& rhs)
{
    std::swap(m_impl,rhs.m_impl);
}
