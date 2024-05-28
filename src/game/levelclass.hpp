//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/platform.hpp"
#include "engine/PFile.hpp"
#include "levelsector.hpp"

#include "tileset.hpp"
#include <vector>
#include <string>

typedef struct {
    int left, top, right, bottom;
} MAP_RECT;

#define PK2MAP_LAST_VERSION "1.3"

#define PK2MAP_MAP_WIDTH  256
#define PK2MAP_MAP_HEIGHT 224
#define PK2MAP_MAP_SIZE   PK2MAP_MAP_WIDTH * PK2MAP_MAP_HEIGHT

#define PK2MAP_MAP_MAX_PROTOTYPES 100

#define SWITCH_INITIAL_VALUE 2000


enum{
    GAME_MODE_STANDARD = 0,
    GAME_MODE_KILL_ALL, //1
    GAME_MODE_CHICK //2
};

class LevelClass {
	private:

	int arrows_block_degree = 0;  // degree of movable blocks
	int tiles_animation_timer  = 0;  // timer for water, fire and waterfall animation
	int block_animation_frame = 0;  // block animations frame
	u32 button1_timer  = 0;  // button 1 timer
	u32 button2_timer  = 0;  // button 2 timer
	u32 button3_timer  = 0;  // button 3 timer

    public:
    /* Atributs ------------------------*/

    char     version[5]       = PK2MAP_LAST_VERSION;         // map version. eg "1.3"

    std::string tileset_name = "blox.bmp";                  // path of block palette .bmp
    std::string tileset_bg_name = "";                       // optional different tileset for the background layer

    std::string background_name = "default.bmp";            // path of map bg .bmp
    std::string music_name = "default.xm";                  // path of map music*/           

    std::string name = "untitled";
    std::string author = "unknown";
    /*char     name[40]   = "untitled";                       // map name
    char     author[40] = "unknown";                        // map author*/

    int      level_number          = 0;                            // level of the episode
    
    int      map_time           = 0;                            // map time (in (dec)conds)
    int       extra          = 0;                            // extra config - not used
    
    u32      button1_time   = SWITCH_INITIAL_VALUE;         // button 1 time
    u32      button2_time   = SWITCH_INITIAL_VALUE;         // button 2 time
    u32      button3_time   = SWITCH_INITIAL_VALUE;         // button 3 time
    int      player_sprite_index = 0;                            // player prototype

    LevelSector sectorPlaceholder;

    /*u8       background_tiles[PK2MAP_MAP_SIZE] = {255};              // map bg tiles 256*224
    u8       foreground_tiles[PK2MAP_MAP_SIZE] = {255};              // map fg tiles 256*224
    u8       sprite_tiles[PK2MAP_MAP_SIZE] = {255};              // map sprites 256*224
    bool     edges [PK2MAP_MAP_SIZE] = {false};            // map edges - calculated during game*/
     
    std::vector<std::string> sprite_prototype_names;        // map prototype list .spr
    int      icon_x = 0;                                         // map icon x pos
	int      icon_y = 0;                                         // map icon x pos
    int      icon_id = 0;                                      // map icon id

    

    std::string lua_script = "main.lua";                        // lua script
    int game_mode = 0;                                          // game mode


    Tileset tileset1; //tilset
    Tileset tileset2; //optional tileset for background
    Background background; //

    std::array<PK2BLOCK, TILESET_SIZE> block_types;
    
    /* Metodit --------------------------*/

    LevelClass(); 
    ~LevelClass();

    void Load(PFile::Path path);
    void Load_Plain_Data(PFile::Path path, bool headerOnly);

    void DrawBackgroundTiles(int camera_x, int camera_y){
        this->sectorPlaceholder.drawBackgroundTiles(camera_x, camera_y, this->block_animation_frame);
    }
    void DrawForegroundTiles(int kamera_x, int kamera_y);

    void SetTilesAnimations(int degree, int anim, u32 aika1, u32 aika2, u32 aika3);

    void Calculate_Edges(){
        this->sectorPlaceholder.calculateEdges();
    }

    void SaveVersion15(PFile::Path path)const;

    void calculateBlockTypes();
    void moveBlocks(u32 button1, u32 button2, u32 button3);

private:
    static void ReadTiles(PFile::RW& file,
        u8 compression,
        u32 level_width,
        std::size_t level_size,
        u8* tiles);

    void LoadVersion13(PFile::Path path, bool headerOnly);
    void LoadVersion15(PFile::Path path, bool headerOnly);
};
