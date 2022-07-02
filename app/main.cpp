#include <string>
#include "DisneyWindow.h"


/*
 *  Initializes GLFW, creates a window, and pumps the render() function.
 */

int main(int /*argc*/,char **argv)
{
    // capture the path to this binary
    
    std::string binaryPath(argv[0]);
    size_t index = binaryPath.find_last_of("\\/");
    
    if(index != std::string::npos)
    {
        binaryPath = binaryPath.substr(0,index + 1);
    }
    
    
    // instantiate the window passing the binary path, so we can find the
    // Disney+ logo
    
    DisneyWindow window(binaryPath);
    
    if(!window.create(1280,720,"Disney+ Project"))
        return 1;
    
    while(window.update())
        window.render();

    return 0;
}
