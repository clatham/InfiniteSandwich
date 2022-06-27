#include <string>
#include "DisneyWindow.h"


/*
 *  Initializes GLFW, creates a window, and pumps the render() function.
 */

int main(int /*argc*/,char **argv)
{
    std::string binaryPath(argv[0]);
    size_t index = binaryPath.find_last_of("\\/");
    
    if(index != std::string::npos)
    {
        binaryPath = binaryPath.substr(0,index + 1);
    }
    
    
    DisneyWindow window(binaryPath);
    
    if(!window.create(1280,720,"Disney+ Project"))
        return 1;
    
    while(window.update())
        window.render();

    return 0;
}
