#pragma once
#include <string>


class Window
{
    public:
        Window();
        virtual ~Window();
        
        bool create(int width,int height,const std::string& title = "");
        void destroy();
        
        bool update();
        void render();
        
        int width() const;
        int height() const;
        
        float opacity() const;
        void setOpacity(float opacity);
        
        void setAspectRatio(int num,int denom);
        
        void setTitle(const std::string& title);
        
        void *handle();
        
        static double time();
        static void setTime(double time);
        
    protected:
        virtual bool onCreate();
        virtual void onDestroy();
        
        virtual void onResize(int width,int height);
        
        virtual void onKeyPress(int key);
        virtual void onKeyRelease(int key);
        virtual void onKeyRepeat(int key);
        
        virtual bool onUpdate();
        virtual void onRender();
        
    private:
        static void resizeCallback(void *handle,int width,int height);
        static void keyCallback(void *handle,int key,int scancode,int action,int mods);
        
        
        struct PrivateImpl;
        PrivateImpl *m_impl;
};
