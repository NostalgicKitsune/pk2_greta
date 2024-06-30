//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################

/**
 * @brief
 * Lua utils by SaturninTheAlien
 */
#include "lua_sprites_list.hpp"

#include "game/prototypes_handler.hpp"
#include "game/sprites_handler.hpp"
#include "game/game.hpp"
#include <vector>

namespace PK2lua{


PrototypeClass* LoadSpritePrototype(const std::string& filename){
    return Game->spritePrototypes.loadPrototype(filename);
}

SpriteClass* AddSprite1(PrototypeClass* prototype, double x, double y, SpriteClass*parent){
    return Game->playerSprite->level_sector->sprites.addLuaSprite(prototype, x, y, parent);
}

SpriteClass* AddSprite2(PrototypeClass* prototype, double x, double y){
    return Game->playerSprite->level_sector->sprites.addLuaSprite(prototype, x, y);
}


void ForEachCreature(sol::object o){

    if(o.is<std::function<void(SpriteClass* s)>>()){
        sol::protected_function func = o;

        for(LevelSector* sector: Game->level.sectors){
            for(SpriteClass*sprite: sector->sprites.Sprites_List){
                if(sprite->energy > 0
                && sprite->prototype->type == TYPE_GAME_CHARACTER
                && !sprite->removed){

                    sol::protected_function_result res = func(sprite);
                    if(!res.valid()){
                        throw res.get<sol::error>();
                    }
                }       
            }
        }
    }
    else{
        throw std::runtime_error("PK2Lua, incorrect lua function!");
    }
}

void ForEachSprite(sol::object o){

    if(o.is<std::function<void(SpriteClass* s)>>()){
        sol::protected_function func = o;

        for(LevelSector* sector: Game->level.sectors){
            for(SpriteClass*sprite: sector->sprites.Sprites_List){
                if(!sprite->removed){

                    sol::protected_function_result res = func(sprite);
                    if(!res.valid()){
                        throw res.get<sol::error>();
                    }
                }   
            }
        }
    }
    else{
        throw std::runtime_error("PK2Lua, incorrect lua function!");
    }
}

void ExposeSpriteListAPI(sol::table& PK2_API){
    /**
     * @brief 
     * Load sprite prototype (.spr2/.spr)
     */

    PK2_API["load_sprite_prototype"] = LoadSpritePrototype;
    /**
     * @brief 
     * Add a new sprite
     */
    PK2_API["add_sprite"] = sol::overload(AddSprite1, AddSprite2);

    /**
     * @brief 
     * Get the player sprite
     */
    PK2_API["get_player"] = [](){return Game->playerSprite;};

    /**
     * @brief 
     * If player is alive and not invisible
     * nil otherwise
     */
    PK2_API["get_player_if_accessible"] = [](){return AI_Functions::player;};

    /**
     * @brief 
     * Execute a Lua function for all the sprites
     */
    PK2_API["for_each_creature"] = ForEachCreature;    
    PK2_API["for_each_sprite"] = ForEachSprite;

    /**
     * @brief 
     * Show info
     */
    PK2_API["show_info"] = [](const std::string& text){ Game->Show_Info(text);};

}

}