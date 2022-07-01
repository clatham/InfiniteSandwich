#pragma once
#include "Image.h"


class Texture
{
    public:
        Texture();
        ~Texture();
        
        bool create(const Image& image);
        void destroy();
        
        bool valid() const;
        
        int width() const;
        int height() const;
        
        void draw(float screenX,float screenY,float screenWidth,float screenHeight,
                  float textureX,float textureY,float textureWidth,float textureHeight);
        
    private:
        struct PrivateImpl;
        PrivateImpl *m_impl;
};
