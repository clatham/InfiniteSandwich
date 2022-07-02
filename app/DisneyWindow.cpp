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
    // force the aspect ratio to 16:9, so we don't have to correct for items
    // elsewhere
    setAspectRatio(16,9);
    
    
    // use the binary path to find the Disney+ logo and create a texture of it
    std::string filename = m_binaryPath + "DisneyPlusLogo.png";
    
    Image image;
    if(image.load(filename))
    {
        m_disneyPlusLogo.create(image);
    }
    
    
    // load the font.  we chose Arial because we know it will always be there
    m_font.load("C:\\Windows\\Fonts\\Arial.ttf",20.0);
    
    
    // request the main JSON file and parse it
    const std::string url = "https://cd-static.bamgrid.com/dp-117731241344/home.json";
    
    m_supplicant.request(url);
    
    nlohmann::json jsonHome = nlohmann::json::parse(m_supplicant.data());
    parseStandardCollection(jsonHome["data"]["StandardCollection"],m_tileSets);
    
    
    // create a worker thread to gather all of the images from the URLs we
    // gathered while parsing the JSON
    m_worker = std::thread(loadTextures,this);
    
    
    // save the start time, and prime a few time variables with it
    m_startTime = time();
    m_currentTime = m_startTime;
    m_selectionChangeTime = m_startTime;
    
    
    // create the selection rectangle
    m_selection.create();
    
    
    // silence all the decoder output except fatal errors
    m_decoder.setLoggingLevel(8);
    return true;
}


void DisneyWindow::onDestroy()
{
    m_decoder.close();
    
    
    m_selection.destroy();
    
    
    m_worker.join();
    
    
    m_tileSets.clear();
    
    
    m_font.destroy();
    
    
    m_disneyPlusLogo.destroy();
}


void DisneyWindow::onKeyPress(int key)
{
    // save selection info, so we can easily tell if it changed
    int prevSelectionRow = m_selectionRow;
    int prevSelectionColumn = m_selectionColumn;
    int prevRowOffset = m_rowOffset;
    int prevColumnOffset = m_tileSets[m_rowOffset + m_selectionRow].columnOffset;
    
    
    // we only move around in a 4x5 grid, but the row and column offsets allow
    // many more off-screen tiles.  therefore, when we move we have to figure
    // out if we're changing the row/column selection or the row/column offset.
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
    
    
    // if the selection changed, restart the 3 second countdown to show video
    if(prevSelectionRow != m_selectionRow  ||
       prevSelectionColumn != m_selectionColumn  ||
       prevRowOffset != m_rowOffset  ||
       prevColumnOffset != m_tileSets[m_rowOffset + m_selectionRow].columnOffset)
    {
        m_selectionChangeTime = m_currentTime;
    }
}


void DisneyWindow::onKeyRepeat(int key)
{
    // forward key repeats to the key press handler
    onKeyPress(key);
}


void DisneyWindow::onRender()
{
    // clear the background to Disney blue (according to Google)
    ::glClearColor(0.08f,0.22f,0.4f,1.0f);
    ::glClear(GL_COLOR_BUFFER_BIT);
    
    
    // if the selection just changed, then go ahead and try to open the new
    // video url
    if(m_selectionChangeTime == m_currentTime  &&
       !m_tileSets[m_selectionRow + m_rowOffset].tiles[m_selectionColumn + m_tileSets[m_selectionRow + m_rowOffset].columnOffset].videoUrl.empty())
    {
        m_decoder.close();
        m_decoder.open(m_tileSets[m_selectionRow + m_rowOffset].tiles[m_selectionColumn + m_tileSets[m_selectionRow + m_rowOffset].columnOffset].videoUrl);
        
        m_videoUpdateTime = m_currentTime;
    }
    
    
    m_currentTime = time();
    
    
    // for the first 2 seconds we display the Disney+ logo to hide the loading
    // of tile images
    if(m_currentTime - m_startTime < 2.0)
    {
        m_disneyPlusLogo.draw(0.0f,0.0f,1.0f,1.0f,
                              0.0f,0.0f,1.0f,1.0f);
    }
    else
    {
        // if we've been on this selection for 3 seconds, it's time to display
        // the next frame, and the video url isn't empty, then try to decode
        // and display the next frame
        if(m_currentTime - m_selectionChangeTime >= 3.0  &&
           m_currentTime >= m_videoUpdateTime  &&
           !m_tileSets[m_selectionRow + m_rowOffset].tiles[m_selectionColumn + m_tileSets[m_selectionRow + m_rowOffset].columnOffset].videoUrl.empty())
        {
            Image image;
            int result = m_decoder.decode(image);
            
            if(result < 0)
            {
                m_decoder.close();
                m_decoder.open(m_tileSets[m_selectionRow + m_rowOffset].tiles[m_selectionColumn + m_tileSets[m_selectionRow + m_rowOffset].columnOffset].videoUrl);
                
                m_videoUpdateTime = m_currentTime;
            }
            else if(result > 0)
            {
                m_videoFrame.destroy();
                m_videoFrame.create(image);
                
                // these videos are all 24fps, so try to acheive that despite
                // our 60fps framework
                m_videoUpdateTime = m_currentTime + 1.0 / 24.0;
            }
        }


        float rowCount = 4;
        float columnCount = 5.5;
        
        float tileWidth = 2.0f / columnCount;
        float tileHeight = 2.0f / rowCount;
        
        m_mutex.lock();
        for(int row = 0;row < (int) rowCount;++row)
        {
            // draw the text caption for each row/tile set
            m_font.drawText(m_tileSets[row + m_rowOffset].name,
                            -1.0f + tileWidth * 0.13f,1.0f - tileHeight * row - tileHeight * 0.125f,0.5f,0.1f);
            
            
            for(int column = 0;column < (int) columnCount + 1;++column)
            {
                // if we're trying to draw the half tile at the right of the
                // row but it doesn't exit, just bail out
                if(column == 5  &&  m_tileSets[row + m_rowOffset].columnOffset == (int) m_tileSets[row + m_rowOffset].tiles.size() - 1 - 4)
                    break;
                
                
                // draw all of the tiles at 90% of the grid width and height,
                // except the selected tile, which is 95%
                float scale = 0.90f;
                if(row == m_selectionRow  &&  column == m_selectionColumn)
                {
                    scale = 0.95f;
                    m_selection.draw(-1.0f + tileWidth * column + tileWidth * 0.6f,1.0f - tileHeight * row - tileHeight * 0.5f,tileWidth * scale,tileWidth * scale);
                }
                
                
                // if this is the selected tile, the video url is valid, and
                // it's been 3 seconds since it was selected, then draw the
                // current video frame
                if(row == m_selectionRow  &&  column == m_selectionColumn  &&
                   !m_tileSets[m_selectionRow + m_rowOffset].tiles[m_selectionColumn + m_tileSets[m_selectionRow + m_rowOffset].columnOffset].videoUrl.empty()  &&
                   m_currentTime - m_selectionChangeTime >= 3.0)
                {
                    m_videoFrame.draw(-1.0f + tileWidth * column + tileWidth * 0.6f,1.0f - tileHeight * row - tileHeight * 0.5f,tileWidth * scale,tileWidth * scale,
                                      0.0f,0.0f,1.0f,1.0f);
                }
                // if the texture for this tile exists, then draw it
                else if(m_tileSets[row + m_rowOffset].tiles[column + m_tileSets[row + m_rowOffset].columnOffset].texture)
                {
                    m_tileSets[row + m_rowOffset].tiles[column + m_tileSets[row + m_rowOffset].columnOffset].texture->draw(-1.0f + tileWidth * column + tileWidth * 0.6f,1.0f - tileHeight * row - tileHeight * 0.5f,tileWidth * scale,tileWidth * scale,
                                                                0.0f,0.0f,1.0f,1.0f);
                }
                // if the image for this tile has been loaded, then create a
                // texture for it
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
                // if everything else failed, then draw the Disney+ logo for
                // this tile
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
}


void DisneyWindow::parseStandardCollection(nlohmann::json& standardCollection,std::vector<TileSet>& tileSets)
{
    // this function will be called with a StandardCollection object, not its
    // parent object
    
    auto containers = standardCollection["containers"];
    
    for(int index = 0;index < containers.size();++index)
    {
        if(containers[index]["set"]["type"].get<std::string>() == "SetRef")
        {
            // if the set is a SetRef, then retrieve its JSON and parse it asctime
            // a normal set
            
            std::string refId = containers[index]["set"]["refId"].get<std::string>();
            std::string url = std::string("https://cd-static.bamgrid.com/dp-117731241344/sets/") + refId + ".json";
            
            m_supplicant.request(url);
            
            
            nlohmann::json jsonRefSet = nlohmann::json::parse(m_supplicant.data());
            
            if(!jsonRefSet.contains("data"))
                return;
            
            parseSet(jsonRefSet["data"].front(),tileSets);
        }
        else
        {
            // this is just a normal set, so parse it
            
            parseSet(containers[index]["set"],tileSets);
        }
    }
}


void DisneyWindow::parseSet(nlohmann::json& set,std::vector<TileSet>& tileSets)
{
    // this function will be called with a set object, not its parent object
    
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
    // this function will run in a separate thread, and it will retrieve the
    // tile images as fast as it can then finish
    
    WebSupplicant supplicant;
    int rowCount = (int) object->m_tileSets.size();
    
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
