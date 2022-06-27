#pragma once
#include <string>


class Image
{
    public:
        Image();
        Image(const Image& rhs);
        Image(Image&& rhs);
        ~Image();
        
        bool load(const std::string& filename);
        bool load(const unsigned char *data,int length);
        bool load(const unsigned char *data,int width,int height,int bitsPerPixel,bool invert = true);
        void destroy();
        
        int width() const;
        int height() const;
        int bitsPerPixel() const;
        
        const void *pixelData() const;
        
        Image& operator=(const Image& rhs);
        Image& operator=(Image&& rhs);
        
        void swap(Image& rhs);
        
    private:
        struct PrivateImpl;
        PrivateImpl *m_impl;
};
