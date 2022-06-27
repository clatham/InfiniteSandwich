#include <iomanip>
#include <iostream>
#include "curl/curl.h"
#include "WebSupplicant.h"
#include "WebServices.h"


#define SetOptionAndReportError(handle,option,value)  \
    {  \
        CURLcode result = ::curl_easy_setopt(handle,option,value);  \
    \
        if(result != CURLE_OK)  \
        {  \
            std::cerr << "WebSupplicant::request:  error setting option:  "  \
                      << ::curl_easy_strerror(result)  \
                      << std::endl;  \
        \
            return false;  \
        }  \
    }


static void hexDump(unsigned char *buffer,int length)
{
    int line = 0;
    
    while(length)
    {
        int bytesToRead = length < 16 ? length : 16;
        
        
        std::cout << std::hex << std::setfill('0');
    
        for(int i = 0;i < bytesToRead;++i)
        {
            if(i != 0)
                std::cout << " ";
            
            std::cout << std::setw(2) << (unsigned int) *buffer++;
            
            if(i == bytesToRead - 1)
                std::cout << "    ";
        }
        
        
        std::cout << std::dec;
        
        for(int i = 0;i < bytesToRead;++i)
        {
            unsigned char c = *buffer++;
            
            if(c < 32  ||  c > 127)
                std::cout << '.';
            else
                std::cout << c;
            
            if(i == bytesToRead - 1)
                std::cout << '\n';
        }
        
        
        length -= bytesToRead;
        ++line;
        
        if(line % 16 == 0)
            std::cout << '\n';
    }
}


static size_t writeCallback(void *contents,size_t size,size_t nmemb,void *opaque)
{
    size_t totalBytes = size * nmemb;
    std::string *data = (std::string *) opaque;
    data->insert(data->end(),(const uint8_t *) contents,(const uint8_t *) contents + totalBytes);
//std::cout << "\n\ntransfer of " << (int) totalBytes << " bytes:\n";
//hexDump((unsigned char *) contents,(int) totalBytes);
    
    return totalBytes;
}


struct WebSupplicant::PrivateImpl
{
    CURL *curl;
    std::string data;
};


WebSupplicant::WebSupplicant() :
    m_impl(new PrivateImpl)
{
    WebServices::start();
    
    
    if(m_impl)
        m_impl->curl = ::curl_easy_init();
}


WebSupplicant::~WebSupplicant()
{
    if(m_impl)
    {
        if(m_impl->curl)
            ::curl_easy_cleanup(m_impl->curl);
        
        delete m_impl;
    }
}


bool WebSupplicant::request(const std::string& url)
{
    if(!m_impl)
        return false;
    
    if(!m_impl->curl)
        return false;
    
    
    m_impl->data.clear();
    
    
    SetOptionAndReportError(m_impl->curl,CURLOPT_URL,url.c_str());
    SetOptionAndReportError(m_impl->curl,CURLOPT_WRITEFUNCTION,writeCallback);
    SetOptionAndReportError(m_impl->curl,CURLOPT_WRITEDATA,(void *) &m_impl->data);
    SetOptionAndReportError(m_impl->curl,CURLOPT_USERAGENT,"libcurl-agent/1.0");
/*  SetOptionAndReportError(m_impl->curl,CURLOPT_FOLLOWLOCATION,true);*/
    SetOptionAndReportError(m_impl->curl,CURLOPT_SSL_VERIFYPEER,false);
    SetOptionAndReportError(m_impl->curl,CURLOPT_SSL_VERIFYHOST,false);

    
    CURLcode result = ::curl_easy_perform(m_impl->curl);
    
    if(result != CURLE_OK)
    {
        std::cerr << "WebSupplicant::request:  error performing request:  "
                  << ::curl_easy_strerror(result)
                  << std::endl;
        
        return false;
    }

    return true;
}


std::string WebSupplicant::data() const
{
    if(!m_impl)
        return std::string();
    
    return m_impl->data;
}
