#pragma once
#include "../core/geometry.h"
#include<Windows.h>
typedef struct mouse {
	// Û±ÍŒª÷√
	vec2 orbit_pos;
	vec2 orbit_deleta;
	vec2 fv_pos;
	vec2 fv_delta;
	float wheel_delta; //πˆ¬÷
}mouse_t;

typedef struct window {
	HWND h_window;
	HDC mem_dc;
	HBITMAP bm_old;
	HBITMAP bm_dib;
	unsigned char* window_fb;
	int width;
	int height;
	char keys[512];
	char buttons[2];
	int is_close;
	mouse_t mouse_info;
}window_t;

extern window_t* window;
int window_init(int width, int height, const char* title);
int window_destroy();
void window_draw(unsigned char* framebuffer);
void msg_dispatch();
vec2 get_mouse_pos();
float platform_get_time(void);