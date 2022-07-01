#pragma once
#include <string>
#include "Image.h"


class VideoDecoder
{
    public:
        VideoDecoder();
        ~VideoDecoder();
        
        bool valid() const;
        
        bool open(const std::string& filename);
        void close();
        
        int decode(Image& image);
        
        int loggingLevel() const;
        void setLoggingLevel(int level);
        
    private:
        struct PrivateImpl;
        PrivateImpl *m_impl;
};
