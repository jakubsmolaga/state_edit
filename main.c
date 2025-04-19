#include "platform.h"
#include "render.h"
#include <unistd.h>

int main() {
	create_window(800,  600, "my window");
	render_init();

	Color blue = {0.2f, 0.2f, 0.8f, 1.0f};
	Color red = {0.8f, 0.2f, 0.2f, 1.0f};
	Color green = {0.2f, 0.8f, 0.2f, 1.0f};
	Color yellow = {0.8f, 0.8f, 0.2f, 1.0f};

	while (get_next_frame()) {
		int width, height;
		get_window_size(&width, &height);
		start_drawing(width, height);
		draw_rect(100, 100, 200, 150, (Color){8.0f, 0.2f, 0.2f, 1.0f});
		draw_rect(400 - 20, 300 + 20, 150 + 10, 200 + 10, (Color){0.0f, 0.0f, 0.0f, 0.5f});
		draw_rect(400 - 2, 300 - 2, 150 + 4, 200 + 4, (Color){0.95f, 0.95f, 0.95f, 1.0f});
		draw_rect_gradient(400, 300, 150, 200, blue, red, green, yellow);

		draw_triangle(100, 500, 300, 500, 300, 800, yellow);
		finish_drawing();

		swap_buffers();
		sleep(1);
	}
	close_window();


	return 0;
}
