#pragma once
#include "Image.h"


class Texture
{
    public:
        Texture();
        Texture(const Texture& rhs);
        Texture(Texture&& rhs);
        ~Texture();
        
        bool create(const Image& image);
        void destroy();
        
        bool valid() const;
        
        int width() const;
        int height() const;
        
        void draw(float screenX,float screenY,float screenWidth,float screenHeight,
                  float textureX,float textureY,float textureWidth,float textureHeight);
        
        Texture& operator=(const Texture& rhs);
        Texture& operator=(Texture&& rhs);
        
        void swap(Texture& rhs);
        
    private:
        struct PrivateImpl;
        PrivateImpl *m_impl;
};
