//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "gfx/particle.hpp"
#include "game/levelclass.hpp"

void Particles_Update();

void Particles_New(int type, double x, double y, double a, double b, int time, double weight, int color);

void Particles_DrawFront(int cam_x, int cam_y);

void Particles_DrawBG(int cam_x, int cam_y);

void Particles_LoadBG(LevelClass* level);

void Particles_Clear();
