local pk2 = require("pk2") -- to access pk2 api

pk2.events.add_listener(pk2.events.GAME_STARTED, function ()
    print("Lua 🌜, Game started!")
    --pk2.start_life();
end )

pk2.events.add_listener(pk2.events.EVENT1, function ()
    pk2.start_life(16);
    pk2.place_rle("gosperglidergun.rle", 123, 160, 4)

end )


local icicle = pk2.load_sprite_prototype("icicle3.spr2")

pk2.events.add_listener(pk2.events.EVENT2, function ()

    local player = pk2.get_player_if_accessible()
    if player~=nil then
        pk2.add_sprite(icicle, player.x, 2048)
    end   
end )