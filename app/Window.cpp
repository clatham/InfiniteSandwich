#include <iostream>
#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "Window.h"
#include "WindowServices.h"


static void APIENTRY debugMessageCallback(GLenum source,GLenum type,unsigned int id,GLenum severity,GLsizei length,const char *message,const void *userParam)
{
    if(severity == GL_DEBUG_SEVERITY_NOTIFICATION  ||  type == GL_DEBUG_TYPE_PERFORMANCE)
        return;
    
    switch(source)
    {
        case GL_DEBUG_SOURCE_API:              std::cout << "Source: API";              break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:    std::cout << "Source: Window System";    break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:  std::cout << "Source: Shader Compiler";  break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:      std::cout << "Source: Third Party";      break;
        case GL_DEBUG_SOURCE_APPLICATION:      std::cout << "Source: Application";      break;
        case GL_DEBUG_SOURCE_OTHER:            std::cout << "Source: Other";            break;
    }
    std::cout << '\n';
    
    switch(type)
    {
        case GL_DEBUG_TYPE_ERROR:                std::cout << "Type: Error";                break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:  std::cout << "Type: Deprecated Behavior";  break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:   std::cout << "Type: Undefined Behavior";   break;
        case GL_DEBUG_TYPE_PORTABILITY:          std::cout << "Type: Portability";          break;
        case GL_DEBUG_TYPE_PERFORMANCE:          std::cout << "Type: Performance";          break;
        case GL_DEBUG_TYPE_MARKER:               std::cout << "Type: Marker";               break;
        case GL_DEBUG_TYPE_PUSH_GROUP:           std::cout << "Type: Push Group";           break;
        case GL_DEBUG_TYPE_POP_GROUP:            std::cout << "Type: Pop Group";            break;
        case GL_DEBUG_TYPE_OTHER:                std::cout << "Type: Other";                break;
    }
    std::cout << '\n';
    
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:          std::cout << "Severity: High";          break;
        case GL_DEBUG_SEVERITY_MEDIUM:        std::cout << "Severity: Medium";        break;
        case GL_DEBUG_SEVERITY_LOW:           std::cout << "Severity: Low";           break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:  std::cout << "Severity: Notification";  break;
    }
    std::cout << '\n';
    std::cout << message << '\n' << std::endl;
}


struct Window::PrivateImpl
{
    GLFWwindow *windowHandle;
};


Window::Window() :
    m_impl(new PrivateImpl)
{
    WindowServices::start();
    
    
    if(m_impl)
        m_impl->windowHandle = NULL;
}


Window::~Window()
{
    if(m_impl)
    {
        if(m_impl->windowHandle)
            destroy();
    
        delete m_impl;
    }
}


bool Window::create(int width,int height,const std::string& title)
{
    if(!m_impl)
        return false;
    
    if(m_impl->windowHandle)
        return false;
    
    
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    ::glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    ::glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,GLFW_TRUE);
    
    m_impl->windowHandle = ::glfwCreateWindow(width,height,title.c_str(),NULL,NULL);
    
    if(!m_impl->windowHandle)
    {
        std::cerr << "Window::create:  error creating window" << std::endl;
        return false;
    }
    
    
    ::glfwSetWindowUserPointer(m_impl->windowHandle,(void *) this);
    
    ::glfwSetFramebufferSizeCallback(m_impl->windowHandle,(GLFWframebuffersizefun) resizeCallback);
    ::glfwSetKeyCallback(m_impl->windowHandle,(GLFWkeyfun) keyCallback);
    
    ::glfwMakeContextCurrent(m_impl->windowHandle);
    ::glfwSwapInterval(1);
    
    
    if(!::gladLoadGLLoader((GLADloadproc) ::glfwGetProcAddress))
    {
        std::cout << "Window::create:  error loading gl function pointers" << std::endl;
        return false;
    }
    
    
    ::glEnable(GL_DEBUG_OUTPUT);
    ::glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    ::glDebugMessageCallback(debugMessageCallback,NULL);
    ::glDebugMessageControl(GL_DONT_CARE,GL_DONT_CARE,GL_DONT_CARE,0,NULL,GL_TRUE);
    
    return onCreate();
}


void Window::destroy()
{
    if(!m_impl)
        return;
    
    if(!m_impl->windowHandle)
        return;
    
    
    onDestroy();
    
    ::glfwDestroyWindow(m_impl->windowHandle);
    m_impl->windowHandle = NULL;
}


bool Window::update()
{
    if(!m_impl)
        return false;
    
    if(!m_impl->windowHandle)
        return false;
    
    
    return onUpdate()  &&  !::glfwWindowShouldClose(m_impl->windowHandle);
}


void Window::render()
{
    if(!m_impl)
        return;
    
    if(!m_impl->windowHandle)
        return;
    
    
    ::glfwMakeContextCurrent(m_impl->windowHandle);
    

    onRender();
    

    ::glfwSwapBuffers(m_impl->windowHandle);
    
    ::glfwPollEvents();
}


int Window::width() const
{
    if(!m_impl)
        return 0;
    
    if(!m_impl->windowHandle)
        return 0;
    
    
    int width, height;
    ::glfwGetFramebufferSize(m_impl->windowHandle,&width,&height);
    
    return width;
}


int Window::height() const
{
    if(!m_impl)
        return 0;
    
    if(!m_impl->windowHandle)
        return 0;
    
    
    int width, height;
    ::glfwGetFramebufferSize(m_impl->windowHandle,&width,&height);
    
    return height;
}


float Window::opacity() const
{
    if(!m_impl)
        return 0.0f;
    
    if(!m_impl->windowHandle)
        return 0.0f;
    
    
    return ::glfwGetWindowOpacity(m_impl->windowHandle);
}


void Window::setOpacity(float opacity)
{
    if(!m_impl)
        return;
    
    if(!m_impl->windowHandle)
        return;
    
    
    ::glfwSetWindowOpacity(m_impl->windowHandle,opacity);
}


void Window::setAspectRatio(int num,int denom)
{
    if(!m_impl)
        return;
    
    if(!m_impl->windowHandle)
        return;
    
    
    ::glfwSetWindowAspectRatio(m_impl->windowHandle,num,denom);
}


void Window::setTitle(const std::string& title)
{
    if(!m_impl)
        return;
    
    if(!m_impl->windowHandle)
        return;
    
    ::glfwSetWindowTitle(m_impl->windowHandle,title.c_str());
}


void *Window::handle()
{
    if(!m_impl)
        return NULL;
    
    if(!m_impl->windowHandle)
        return NULL;
    
    return m_impl->windowHandle;
}


double Window::time()
{
    return ::glfwGetTime();
}


void Window::setTime(double time)
{
    ::glfwSetTime(time);
}


bool Window::onCreate()
{
    return true;
}


void Window::onDestroy()
{
}


void Window::onResize(int width,int height)
{
}


void Window::onKeyPress(int key)
{
}


void Window::onKeyRelease(int key)
{
}


void Window::onKeyRepeat(int key)
{
}


bool Window::onUpdate()
{
    return true;
}


void Window::onRender()
{
    ::glClearColor(0.45f,0.55f,0.6f,1.0f);
    ::glClear(GL_COLOR_BUFFER_BIT);
}


void Window::resizeCallback(void *handle,int width,int height)
{
/*    void *opaque = ::glfwGetWindowUserPointer(windowHandle);
    
    if(!opaque)
        return;
    
    Window *window = (Window *) opaque;
    */
    ::glViewport(0,0,width,height);
//    window->render();
    
    void *opaque = ::glfwGetWindowUserPointer((GLFWwindow *) handle);
    
    if(!opaque)
        return;
    
    Window *window = (Window *) opaque;
    window->onResize(width,height);
}


void Window::keyCallback(void *handle,int key,int scancode,int action,int mods)
{
    void *opaque = ::glfwGetWindowUserPointer((GLFWwindow *) handle);
    
    if(!opaque)
        return;
    
    Window *window = (Window *) opaque;
    
    if(action == GLFW_PRESS)
        window->onKeyPress(key);
    else if(action == GLFW_RELEASE)
        window->onKeyRelease(key);
    else if(action == GLFW_REPEAT)
        window->onKeyRepeat(key);
}
