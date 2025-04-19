#pragma once

typedef struct {
	float r, g, b, a;
} Color;

void render_init(void);
void start_drawing(int window_width, int window_height);
void finish_drawing(void);

void draw_rect_gradient(float x, float y, float w, float h, Color c0, Color c1, Color c2, Color c3);
void draw_rect(float x, float y, float w, float h, Color color);
void draw_triangle(float x0, float y0, float x1, float y1, float x2, float y2, Color color);
void draw_line(float x0, float y0, float x1, float y1, float width, Color color);
void draw_circle(float x, float y, float radius, Color color);
void draw_text(const char *text, float height, float x, float y, Color color);
