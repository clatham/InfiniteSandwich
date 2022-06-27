#pragma once
#include <string>


class Font
{
    public:
        Font();
        ~Font();
        
        bool valid() const;
        
        bool load(const std::string& filename,float lineHeight);
        void destroy();
        
        void drawText(const std::string& text,
                      float screenX,float screenY,float screenWidth,float screenHeight);
        
    protected:
        struct PrivateImpl;
        PrivateImpl *m_impl;
};
