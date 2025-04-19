#pragma once

typedef struct {
	float r, g, b, a;
} Color;

void render_init(void);
void start_drawing(int window_width, int window_height);
void finish_drawing(void);

void draw_rect_gradient(float x, float y, float w, float h, Color c0, Color c1, Color c2, Color c3);
void draw_rect(float x, float y, float w, float h, Color color);
