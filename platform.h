#pragma once

void create_window(int width, int height, const char *title);
int get_next_frame(void);
void swap_buffers(void);
void get_window_size(int *width, int *height);
void close_window(void);

