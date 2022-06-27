#pragma once


class WindowServices
{
    public:
        static void start();
    
    private:
        WindowServices();
        ~WindowServices();
        
        static void errorCallback(int error,const char *description);
};
