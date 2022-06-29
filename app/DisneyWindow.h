#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include "Font.h"
#include "nlohmann/json.hpp"
#include "Rectangle.h"
#include "Texture.h"
#include "WebSupplicant.h"
#include "Window.h"


class DisneyWindow : public Window
{
    private:
        struct Tile
        {
            std::string name;
            std::string releaseDate;
            std::string rating;
            std::string url;
            std::string videoUrl;
            std::shared_ptr<Texture> texture;
        };
        
        struct TileSet
        {
            std::string name;
            std::vector<Tile> tiles;
            int columnOffset;
        };
        
    public:
        DisneyWindow(std::string binaryPath);
        ~DisneyWindow();
        
    protected:
        bool onCreate();
        void onDestroy();
        
        void onKeyPress(int key);
        void onKeyRepeat(int key);
        
        void onRender();
    
    private:
        void parseStandardCollection(nlohmann::json& standardCollection,std::vector<TileSet>& tileSets);
        void parseSet(nlohmann::json& set,std::vector<TileSet>& tileSets);
        
        static void loadTextures(DisneyWindow *object);
    
    private:
        std::string m_binaryPath;
        Texture m_disneyPlusLogo;
        
        Font m_font;
        
        WebSupplicant m_supplicant;
        std::vector<TileSet> m_tileSets;
        
        std::mutex m_mutex;
        std::unordered_map<std::string,std::shared_ptr<Image>> m_images;

        std::thread m_worker;
        
        int m_rowOffset;
        int m_selectionRow;
        int m_selectionColumn;
        Rectangle m_selection;
};
