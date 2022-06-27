#include <iostream>
#include "curl/curl.h"
#include "WebServices.h"


WebServices::WebServices()
{
    CURLcode result = ::curl_global_init(CURL_GLOBAL_DEFAULT);
    
    if(result != CURLE_OK)
        std::cerr << "WebServices::WebServices:  error initializing CURL:  "
                  << ::curl_easy_strerror(result)
                  << std::endl;
}


WebServices::~WebServices()
{
    ::curl_global_cleanup();
}


void WebServices::start()
{
    static WebServices services;
}
