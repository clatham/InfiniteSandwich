#pragma once
#include <cstdint>
#include <string>


class WebSupplicant
{
    public:
        WebSupplicant();
        ~WebSupplicant();
        
        bool request(const std::string& url);
        
        std::string data() const;
        
    private:
        struct PrivateImpl;
        PrivateImpl *m_impl;
};
