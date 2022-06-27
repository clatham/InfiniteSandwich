#pragma once


class Rectangle
{
    public:
        Rectangle();
        ~Rectangle();
        
        bool create();
        void destroy();
        
        void draw(float centerX,float centerY,float width,float height);
        
    private:
        struct PrivateImpl;
        PrivateImpl *m_impl;
};
