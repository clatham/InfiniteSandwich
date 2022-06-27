#include <algorithm>
#include <cmath>
#include <iostream>
#include "DisneyWindow.h"
#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "WebSupplicant.h"


DisneyWindow::DisneyWindow(std::string binaryPath) :
    m_binaryPath(binaryPath),
    m_rowOffset(0),
    m_selectionRow(0),
    m_selectionColumn(0)
{
}


DisneyWindow::~DisneyWindow()
{
}


bool DisneyWindow::onCreate()
{
    setAspectRatio(16,9);
    
    
    std::string filename = m_binaryPath + "DisneyPlusLogo.png";
    
    Image image;
    if(image.load(filename))
    {
        m_disneyPlusLogo.create(image);
    }
    
    
    m_font.load("C:\\Windows\\Fonts\\Arial.ttf",20.0);
    
    
    const std::string url = "https://cd-static.bamgrid.com/dp-117731241344/home.json";
    
    WebSupplicant supplicant;
    supplicant.request(url);
    

    nlohmann::json jsonHome = nlohmann::json::parse(supplicant.data());
    parseStandardCollection(jsonHome["data"]["StandardCollection"],m_tileSets);
    
    
    m_worker = std::thread(loadTextures,this);
    
    m_selection.create();
    
    return true;
}


void DisneyWindow::onDestroy()
{
    m_selection.destroy();
    
    m_worker.join();
    
    m_tileSets.clear();
    
    m_font.destroy();
    
    m_disneyPlusLogo.destroy();
}


void DisneyWindow::onKeyPress(int key)
{
    switch(key)
    {
        case GLFW_KEY_RIGHT:
            if(m_selectionColumn == 4)
                m_tileSets[m_rowOffset + m_selectionRow].columnOffset = std::min((int) m_tileSets[m_rowOffset + m_selectionRow].tiles.size() - 1 - 4,m_tileSets[m_rowOffset + m_selectionRow].columnOffset + 1);
            
            m_selectionColumn = std::min(4,m_selectionColumn + 1);
            break;
        
        case GLFW_KEY_LEFT:
            if(m_selectionColumn == 0)
                m_tileSets[m_rowOffset + m_selectionRow].columnOffset = std::max(0,m_tileSets[m_rowOffset + m_selectionRow].columnOffset - 1);
            
            m_selectionColumn = std::max(0,m_selectionColumn - 1);
            break;

        case GLFW_KEY_DOWN:
            if(m_selectionRow == 3)
                m_rowOffset = std::min((int) m_tileSets.size() - 1 - 3,m_rowOffset + 1);

            m_selectionRow = std::min(3,m_selectionRow + 1);
            break;
        
        case GLFW_KEY_UP:
            if(m_selectionRow == 0)
                m_rowOffset = std::max(0,m_rowOffset - 1);

            m_selectionRow = std::max(0,m_selectionRow - 1);
            break;
    }
}


void DisneyWindow::onKeyRepeat(int key)
{
    onKeyPress(key);
}


void DisneyWindow::onRender()
{
    ::glClearColor(0.08f,0.22f,0.4f,1.0f);
    ::glClear(GL_COLOR_BUFFER_BIT);
    
    
    static const double startTime = time();
    double currentTime = time();

    static int frameCount = 0;
    static double lastFpsTime = startTime;
    double deltaTime = currentTime - lastFpsTime;
    
    if(deltaTime >= 1.0)
    {
        double fps = frameCount / deltaTime;
        
        frameCount = 0;
        lastFpsTime = currentTime;
        
        setTitle(std::string("Disney+ Project - [") + std::to_string(fps) + "]");
    }
    
    
    if(currentTime - startTime < 2.0)
    {
        m_disneyPlusLogo.draw(0.0f,0.0f,1.0f,1.0f,
                              0.0f,0.0f,1.0f,1.0f);
    }
    else
    {
        float rowCount = 4;
        float columnCount = 5.5;
        
        float tileWidth = 2.0f / columnCount;
        float tileHeight = 2.0f / rowCount;
        
        m_mutex.lock();
        for(int row = 0;row < (int) rowCount;++row)
        {
            m_font.drawText(m_tileSets[row + m_rowOffset].name,
                            -1.0f + tileWidth * 0.13f,1.0f - tileHeight * row - tileHeight * 0.125f,0.5f,0.1f);
                            
            for(int column = 0;column < (int) columnCount + 1;++column)
            {
                if(column == 5  &&  m_tileSets[row + m_rowOffset].columnOffset == (int) m_tileSets[row + m_rowOffset].tiles.size() - 1 - 4)
                    break;
                
                float scale = 0.90f;
                if(row == m_selectionRow  &&  column == m_selectionColumn)
                {
                    scale = 0.95f;
                    m_selection.draw(-1.0f + tileWidth * column + tileWidth * 0.6f,1.0f - tileHeight * row - tileHeight * 0.5f,tileWidth * scale * 1.005f,tileWidth * scale * 1.005f);
                }
                
                if(m_tileSets[row + m_rowOffset].tiles[column + m_tileSets[row + m_rowOffset].columnOffset].texture)
                {
                    m_tileSets[row + m_rowOffset].tiles[column + m_tileSets[row + m_rowOffset].columnOffset].texture->draw(-1.0f + tileWidth * column + tileWidth * 0.6f,1.0f - tileHeight * row - tileHeight * 0.5f,tileWidth * scale,tileWidth * scale,
                                                                0.0f,0.0f,1.0f,1.0f);
                }
                else if(m_images.find(m_tileSets[row + m_rowOffset].tiles[column + m_tileSets[row + m_rowOffset].columnOffset].url) != m_images.end())
                {
                    std::shared_ptr<Texture> texture = std::make_shared<Texture>();
                    
                    if(texture->create(*m_images[m_tileSets[row + m_rowOffset].tiles[column + m_tileSets[row + m_rowOffset].columnOffset].url]))
                    {
                        m_tileSets[row + m_rowOffset].tiles[column + m_tileSets[row + m_rowOffset].columnOffset].texture = std::move(texture);
                        m_tileSets[row + m_rowOffset].tiles[column + m_tileSets[row + m_rowOffset].columnOffset].texture->draw(-1.0f + tileWidth * column + tileWidth * 0.6f,1.0f - tileHeight * row - tileHeight * 0.5f,tileWidth * scale,tileWidth * scale,
                                                                    0.0f,0.0f,1.0f,1.0f);
                    }
                }
                else
                {
                    m_disneyPlusLogo.draw(-1.0f + tileWidth * column + tileWidth * 0.6f,1.0f - tileHeight * row - tileHeight * 0.5f,tileWidth * scale,tileWidth * scale,
                                          0.0f,0.0f,1.0f,1.0f);
                }
            }
        }
        m_mutex.unlock();
    }
    
    
//    m_font.drawText(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",
//                    -1.0f,0.0f,2.0f,0.25f);
    
    
    ++frameCount;
}


void DisneyWindow::parseStandardCollection(nlohmann::json& standardCollection,std::vector<TileSet>& tileSets)
{
    auto containers = standardCollection["containers"];
    
    for(int index = 0;index < containers.size();++index)
    {
        if(containers[index]["set"]["type"].get<std::string>() == "SetRef")
        {
            std::string refId = containers[index]["set"]["refId"].get<std::string>();
            std::string url = std::string("https://cd-static.bamgrid.com/dp-117731241344/sets/") + refId + ".json";
            
            WebSupplicant supplicant;
            supplicant.request(url);
            
            
            nlohmann::json jsonRefSet = nlohmann::json::parse(supplicant.data());
            
            if(!jsonRefSet.contains("data"))
                return;
            
            parseSet(jsonRefSet["data"].front(),tileSets);
        }
        else
        {
            parseSet(containers[index]["set"],tileSets);
        }
    }
}


void DisneyWindow::parseSet(nlohmann::json& set,std::vector<TileSet>& tileSets)
{
    TileSet tileSet;
    tileSet.name = set["text"]["title"]["full"]["set"]["default"]["content"].get<std::string>();
    tileSet.columnOffset = 0;
    
    auto items = set["items"];
    
    for(int index = 0;index < items.size();++index)
    {
        Tile tile;
        tile.name = items[index]["text"]["title"]["full"].front()["default"]["content"].get<std::string>();
        
        if(!items[index]["ratings"][0]["value"].is_null())
            tile.rating = items[index]["ratings"][0]["value"].get<std::string>();
        
        if(!items[index]["releases"][0]["releaseDate"].is_null())
            tile.releaseDate = items[index]["releases"][0]["releaseDate"].get<std::string>();
        
        tile.url = items[index]["image"]["tile"]["1.78"].front()["default"]["url"].get<std::string>();
        
        if(items[index]["videoArt"].size())
            tile.videoUrl = items[index]["videoArt"][0]["mediaMetadata"]["urls"][0]["url"].get<std::string>();
        
        tileSet.tiles.push_back(tile);
    }
    
    tileSets.push_back(tileSet);
}


void DisneyWindow::loadTextures(DisneyWindow *object)
{
    int rowCount = (int) object->m_tileSets.size();
    
    for(int row = 0;row < 4;++row)
    {
        int columnCount = (int) object->m_tileSets[row].tiles.size();
        
        for(int column = 0;column < 6;++column)
        {
            object->m_mutex.lock();
            auto imageIndex = object->m_images.find(object->m_tileSets[row].tiles[column].url);
            object->m_mutex.unlock();
            
            if(imageIndex == object->m_images.end())
            {
                WebSupplicant supplicant;
                if(supplicant.request(object->m_tileSets[row].tiles[column].url))
                {
                    std::string data = supplicant.data();
                
                    std::shared_ptr<Image> image = std::make_shared<Image>();
                    if(image->load((unsigned char *) data.data(),(int) data.size()))
                    {
                        object->m_mutex.lock();
                        object->m_images[object->m_tileSets[row].tiles[column].url] = std::move(image);
                        object->m_mutex.unlock();
                    }
                }
            }
        }
    }

    for(int row = 0;row < rowCount;++row)
    {
        int columnCount = (int) object->m_tileSets[row].tiles.size();
        
        for(int column = 0;column < columnCount;++column)
        {
            object->m_mutex.lock();
            auto imageIndex = object->m_images.find(object->m_tileSets[row].tiles[column].url);
            object->m_mutex.unlock();
            
            if(imageIndex == object->m_images.end())
            {
                WebSupplicant supplicant;
                if(supplicant.request(object->m_tileSets[row].tiles[column].url))
                {
                    std::string data = supplicant.data();
                
                    std::shared_ptr<Image> image = std::make_shared<Image>();
                    if(image->load((unsigned char *) data.data(),(int) data.size()))
                    {
                        object->m_mutex.lock();
                        object->m_images[object->m_tileSets[row].tiles[column].url] = std::move(image);
                        object->m_mutex.unlock();
                    }
                }
            }
        }
    }
    
    std::cout << "worker thread finished" << std::endl;
}
