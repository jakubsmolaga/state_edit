#pragma once
#include <stdbool.h>

// currently only keys that we need
// we can add more later if needed
typedef enum {
	KEY_UNKNOWN,
	KEY_ARROW_LEFT,
	KEY_ARROW_RIGHT,
	KEY_BACKSPACE,
	KEY_CONTROL,
	KEY_SHIFT,
} SpecialKey;

typedef struct {
	bool shift   : 1;
	bool control : 1;
	bool alt     : 1;
} KeyMods;

typedef enum {
	EVENT_NONE,
	EVENT_KEY_PRESS,
	EVENT_QUIT,
} EventType;

typedef struct {
	EventType type;
	SpecialKey special_key;
	char text[16];
	KeyMods mods;
} KeyEvent;

typedef union {
	EventType type;
	KeyEvent keypress;
} PlatformEvent;

void create_window(int width, int height, const char *title);
void swap_buffers(void);
void get_window_size(int *width, int *height);
void close_window(void);
PlatformEvent platform_next_event(void);
