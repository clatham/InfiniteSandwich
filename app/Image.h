#pragma once
#include <string>


class Image
{
    public:
        Image();
        ~Image();
        
        bool valid() const;
        
        bool load(const std::string& filename);
        bool load(const unsigned char *data,int length);
        bool load(const unsigned char *data,int width,int height,int bitsPerPixel,bool invert = true);
        void destroy();
        
        int width() const;
        int height() const;
        int bitsPerPixel() const;
        
        const void *pixelData() const;
        
    private:
        struct PrivateImpl;
        PrivateImpl *m_impl;
};
