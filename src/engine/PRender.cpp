//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "engine/PRender.hpp"
#include <stdexcept>

#include "engine/PDraw.hpp"
#include "engine/PLog.hpp"
#include "engine/types.hpp"

#include "engine/render/PSdl.hpp"

#include <SDL.h>
#include <SDL_image.h>

namespace PRender {

static int fullscreen_mode = SDL_WINDOW_FULLSCREEN_DESKTOP; //SDL_WINDOW_FULLSCREEN;

static int current_shader = SHADER_LINEAR;

static float cover_width, cover_height;

static bool screen_fill = false;
static FRECT screen_dest = {0.f, 0.f, 1.f, 1.f};

static const char* window_name;
static SDL_Window* window = NULL;
static bool vsync_set = false;

static Renderer* renderer;

void load_ui_texture(PFile::Path file) {

	try{
		PFile::RW rw = file.GetRW2("r");
		SDL_Surface* surface = IMG_Load_RW((SDL_RWops*)(rw._rwops), 0);
		renderer->load_ui_texture(surface);
	}
	catch(const PFile::PFileException& e){
		PLog::Write(PLog::ERR,"PRender", e.what());
		throw std::runtime_error("Cannot load touchscreen texture!");
	}
}

void render_ui(FRECT src, FRECT dst, float alpha) {
	renderer->render_ui(src, dst, alpha);
}

void set_screen_fill(bool set) {

	screen_fill = set;
	adjust_screen();

}

void adjust_screen() {

	if (screen_fill) {

		screen_dest.w = 1.f;
		screen_dest.h = 1.f;
		screen_dest.x = 0.f;
		screen_dest.y = 0.f;

		renderer->set_screen(screen_dest);
		return;

	}

	int w, h;
	SDL_GetWindowSize(window, &w, &h);

	int buf_w, buf_h;
	PDraw::get_buffer_size(&buf_w, &buf_h);

	float screen_prop = (float)w / h;
	float buff_prop   = (float)buf_w / buf_h;

	if (buff_prop > screen_prop) {

		screen_dest.w = 1.f;
		screen_dest.h = screen_prop / buff_prop;
		screen_dest.x = 0.f;
		screen_dest.y = (1.f - screen_dest.h) / 2;
	
	} else {

		screen_dest.w = buff_prop / screen_prop;
		screen_dest.h = 1.f;
		screen_dest.x = (1.f - screen_dest.w) / 2;
		screen_dest.y = 0.f;

	}

	cover_width = screen_dest.w * w;
	cover_height = screen_dest.h * h;

	renderer->set_screen(screen_dest);

}

void set_fullscreen(bool set) {

	#ifdef __ANDROID__
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		return;
	#endif

	renderer->clear_screen();

	if(set)
		SDL_SetWindowFullscreen(window, fullscreen_mode);
	else {
		int buf_w, buf_h;
		PDraw::get_buffer_size(&buf_w, &buf_h);

		SDL_SetWindowFullscreen(window, 0);
		SDL_SetWindowSize(window, buf_w, buf_h);
		SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		//TODO - adjust dst_rect too and turn off filters
	}

	adjust_screen();

}

int set_shader(int shader) {

	if (shader == current_shader)
		return 2;

	int ret = renderer->set_shader(shader);

	if (ret == 0)
		current_shader = shader;

	return ret;

}

void set_window_size(int w, int h) {

	SDL_SetWindowSize(window, w, h);
	adjust_screen();

}

void get_window_size(int* w, int* h) {

	SDL_GetWindowSize(window, w, h);

}

void get_cover_size(int* w, int* h) {

	*w = cover_width;
	*h = cover_height;

}

void get_window_position(int* x, int* y) {

	SDL_GetWindowPosition(window, x, y);

}

int set_vsync(bool set) {

	if (set == vsync_set)
		return 0;

	int ret = renderer->set_vsync(set);

	if (ret == 0)
		vsync_set = set;
	
	return ret;

}

bool is_vsync() {

	return vsync_set;
	
}

int init(int width, int height, const char* name, const char* icon) {

	window_name = name;
	Uint32 window_flags = SDL_WINDOW_SHOWN;

	PLog::Write(PLog::DEBUG, "PRender", "Initializing graphics");
	PLog::Write(PLog::DEBUG, "PRender", "Video driver: %s", SDL_GetCurrentVideoDriver());

    #ifdef __ANDROID__

    window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

    window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
	if (!window) {

		PLog::Write(PLog::FATAL, "PRender", "Couldn't create window!");
		return -2;

	}

    #else

	window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
	if (!window) {

		PLog::Write(PLog::FATAL, "PRender", "Couldn't create window!");
		return -2;

	}

	SDL_Surface* window_icon = IMG_Load(icon);
	if (window_icon) {
		SDL_SetWindowIcon(window, window_icon);
		SDL_FreeSurface(window_icon);
	}
	
	SDL_ShowCursor(SDL_DISABLE);
	
	#endif

	renderer = new PSdl(width, height, window);
	if (!renderer) {

		PLog::Write(PLog::FATAL, "PRender", "Couldn't create renderer!");
		SDL_DestroyWindow(window);
		return -3;

	}

	renderer->set_vsync(vsync_set);
	adjust_screen();

	return 0;

}

void terminate() {

	SDL_DestroyWindow(window);

	delete renderer;
	renderer = nullptr;

}

void update(void* _buffer8) {

	renderer->update(_buffer8);

}

}