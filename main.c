#include "platform.h"
#include "render.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void update(void);
static void draw(void);

int main() {
	create_window(800,  600, "my window");
	render_init();

	while (true) {
		update();
		draw();
		swap_buffers();
		sleep(1);
	}
}

static void
update(void)
{
	PlatformEvent event = platform_next_event();
	while (event.type != EVENT_NONE) {
		if (event.type == EVENT_QUIT) {
			close_window();
			exit(0);
		}
		if (event.type == EVENT_KEY_PRESS) {
			KeyEvent key_event = event.keypress;
			if (key_event.c != 0) {
				printf("key pressed: %c (%d)\n", key_event.c, (int)key_event.c);
			} else {
				printf("special key pressed: %d\n", key_event.special_key);
			}
		}
		event = platform_next_event();
	}
}

static void
draw(void)
{
	Color blue = {0.2f, 0.2f, 0.8f, 1.0f};
	Color red = {0.8f, 0.2f, 0.2f, 1.0f};
	Color green = {0.2f, 0.8f, 0.2f, 1.0f};
	Color yellow = {0.8f, 0.8f, 0.2f, 1.0f};
	Color white = {1.0, 1.0, 1.0, 1.0};
	Color black = {0.0, 0.0, 0.0, 1.0};

	int width, height;
	get_window_size(&width, &height);

	start_drawing(width, height);
	draw_rect(100, 100, 200, 150, (Color){8.0f, 0.2f, 0.2f, 1.0f});
	draw_rect(400 - 20, 300 + 20, 150 + 10, 200 + 10, (Color){0.0f, 0.0f, 0.0f, 0.5f});
	draw_rect(400 - 2, 300 - 2, 150 + 4, 200 + 4, (Color){0.95f, 0.95f, 0.95f, 1.0f});
	draw_rect_gradient(400, 300, 150, 200, blue, red, green, yellow);

	draw_line(100, 500, 300, 800, 10, green);
	draw_triangle(100, 500, 300, 500, 300, 800, yellow);

	draw_circle(400, 300, 50, (Color){0.2f, 0.2f, 0.8f, 1.0f});
	draw_circle(420, 320, 50, red);

	{
		const char *text = "hello world\nhow are you?";
		float font_size = 40.0f;
		float w, h;
		measure_text(text, font_size, &w, &h);
		draw_rect(100 - 10, 300 - 10, w + 20, h + 20, white);
		draw_rect(100 - 8, 300 - 8, w + 16, h + 16, black);
		draw_text(text, 40, 100, 300, white);
	}

	draw_arrow(200, 700, 600, 500, 5, red);

	finish_drawing();
}
