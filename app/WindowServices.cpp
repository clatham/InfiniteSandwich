#include <iostream>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "WindowServices.h"


WindowServices::WindowServices()
{
    ::glfwSetErrorCallback(errorCallback);
    
    if(!::glfwInit())
        std::cerr << "WindowServices::WindowServices:  error initializing GLFW" << std::endl;
}


WindowServices::~WindowServices()
{
    ::glfwTerminate();
}


void WindowServices::start()
{
    static WindowServices services;
}


void WindowServices::errorCallback(int error,const char *description)
{
    std::cerr << "WindowServices::errorCallback:  error " << error << ":  " << description << std::endl;
}
