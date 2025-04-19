#include <GLES3/gl3.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static const char vs_code[] = 
	"#version 300 es\n"
	"precision mediump float;"
	"layout(location = 0) in vec2 a_pos;"
	"layout(location = 1) in vec2 a_uv;"
	"layout(location = 2) in vec4 a_color;"
	"layout(location = 3) in vec2 a_size;"
	"layout(location = 4) in float a_radius;"
	"layout(location = 5) in vec2 a_rect_pos;"
	"uniform vec2 u_resolution;"
	"out vec4 v_color;"
	"out vec2 v_uv;"
	"out vec2 v_pos;"
	"out vec2 v_size;"
	"out float v_radius;"
	"out vec2 v_rect_pos;"
	"void main()"
	"{"
		"vec2 pos = a_pos / u_resolution * 2.0 - 1.0;"
		"pos.y *= -1.0;"
		"gl_Position = vec4(pos, 0.0, 1.0);"
		"v_color = a_color;"
		"v_uv = a_uv;"
		"v_size = a_size;"
		"v_radius = a_radius;"
		"v_rect_pos = a_rect_pos;"
	"}";

static const char fs_code[] =
	"#version 300 es\n"
	"precision mediump float;"
	"in vec4 v_color;"
	"in vec2 v_uv;"
	"in vec2 v_size;"
	"in float v_radius;"
	"in vec2 v_rect_pos;"
	"out vec4 FragColor;"
	"uniform sampler2D texture1;"
	"uniform vec2 u_resolution;"
	"void main()"
	"{"
		"vec2 minB = vec2(v_radius);"
		"vec2 maxB = v_size - v_radius;"
		"vec2 nearest = clamp(v_rect_pos, minB, maxB);"
		"float dist2 = dot(v_rect_pos - nearest, v_rect_pos - nearest);"
		"float mask = step(dist2, v_radius * v_radius);"
		"vec4 col = texture(texture1, v_uv);"
		"FragColor = col * v_color * vec4(1.0, 1.0, 1.0, mask);"
	"}";

typedef struct {
	float x, y;
} Vec2;

typedef struct {
	float r, g, b, a;
} Color;

typedef struct {
	Vec2 pos;
	Vec2 uv;
	Color color;
	Vec2 size;
	float radius;
	Vec2 rect_pos;
} Vertex;


typedef struct {
	float x, y, w, h;
} Rect;

// data required for drawing rectangles
typedef struct {
	Rect src;
	Rect dst;
	Color c0, c1, c2, c3;
	float radius;
} QuadData;

static const float PI = 3.14159265358979323846f;

static Vertex vertices[4096];
static size_t vertex_count = 0;
static GLuint program;
static GLuint vao;
static GLuint vbo;

struct {
	GLuint texture;
	int w, h, cell_w, cell_h, rows, cols;
	Vec2 white_pixel; // pixel used for drawing shapes
} font_atlas;

static float
vec2_len(Vec2 v)
{
        return sqrtf(v.x*v.x + v.y*v.y);
}

static Vec2
vec2_scale(Vec2 v, float scale)
{
        return (Vec2){ v.x * scale, v.y * scale };
}

static float
vec2_get_angle(Vec2 v)
{
        return atan2f(v.y, v.x);
}

static Vec2
vec2_rotate(Vec2 v, float angle)
{
        float cos_a = cosf(angle);
        float sin_a = sinf(angle);
        return (Vec2){ v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a };
}

static Vec2
vec2_norm(Vec2 v)
{
        return vec2_scale(v, 1.0 / vec2_len(v));
}

static void
init_font_atlas(void)
{
	unsigned char *data = stbi_load("font_atlas.png", &font_atlas.w, &font_atlas.h, NULL, 4);
	if (!data) {
		fprintf(stderr, "Failed to load font atlas\n");
		exit(EXIT_FAILURE);
		return;
	}

	glGenTextures(1, &font_atlas.texture);
	glBindTexture(GL_TEXTURE_2D, font_atlas.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font_atlas.w, font_atlas.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	font_atlas.cell_w = 19;
	font_atlas.cell_h = 32;
	font_atlas.rows = 8;
	font_atlas.cols = 13;
	font_atlas.white_pixel.x = 199;
	font_atlas.white_pixel.y = 13;
}

static GLuint
compile_shader_unit(const char *code, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &code, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint log_length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
		char *log = malloc(log_length);
		glGetShaderInfoLog(shader, log_length, NULL, log);
		fprintf(stderr, "Shader compile error: %s\n", log);
		free(log);
		exit(EXIT_FAILURE);
	}

	return shader;
}

static void
init_shader_program(void)
{
	GLuint vs = compile_shader_unit(vs_code, GL_VERTEX_SHADER);
	GLuint fs = compile_shader_unit(fs_code, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint log_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
		char *log = malloc(log_length);
		glGetProgramInfoLog(program, log_length, NULL, log);
		fprintf(stderr, "Program link error: %s\n", log);
		free(log);
		exit(EXIT_FAILURE);
	}

	glDeleteShader(vs);
	glDeleteShader(fs);
}

static void
init_vao(void)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, size));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, radius));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, rect_pos));
	glEnableVertexAttribArray(5);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

static void
flush_vertices(void)
{
	if (vertex_count == 0) {
		return;
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(Vertex), vertices);
	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	vertex_count = 0;
}

static void
push_vertex(Vertex v)
{
	if (vertex_count >= sizeof(vertices) / sizeof(vertices[0])) {
		flush_vertices();
	}
	vertices[vertex_count++] = v;
}

static void
push_simple_vertex(float x, float y, Color color)
{
        Vec2 uv = {
                .x = (font_atlas.white_pixel.x * 1.0f) / font_atlas.w,
                .y = (font_atlas.white_pixel.y * 1.0f) / font_atlas.h,
        };
        Vertex v = {
                .pos = { x, y },
                .uv = uv,
                .color = color,
                .size = { 0, 0 },
                .radius = 0,
                .rect_pos = { 0, 0 },
        };
        push_vertex(v);
}

static void
draw_quad(QuadData q)
{
	if (vertex_count >= sizeof(vertices) / sizeof(vertices[0]) - 6) {
		flush_vertices();
	}
	Vertex v1 = { 
		.pos = { q.dst.x, q.dst.y }, 
		.uv = { q.src.x, q.src.y },
		.color = q.c0, 
		.size = { q.dst.w, q.dst.h },
		.radius = q.radius,
		.rect_pos = { 0, 0 },
	};
	Vertex v2 = { 
		.pos = { q.dst.x + q.dst.w, q.dst.y }, 
		.uv = { q.src.x + q.src.w, q.src.y }, 
		.color = q.c1, 
		.size = { q.dst.w, q.dst.h },
		.radius = q.radius,
		.rect_pos = { q.dst.w, 0 },
	};
	Vertex v3 = { 
		.pos = { q.dst.x + q.dst.w, q.dst.y + q.dst.h },
		.uv = { q.src.x + q.src.w, q.src.y + q.src.h },
		.color = q.c2, 
		.size = { q.dst.w, q.dst.h },
		.radius = q.radius,
		.rect_pos = { q.dst.w, q.dst.h },
	};
	Vertex v4 = { 
		.pos = { q.dst.x + q.dst.w, q.dst.y + q.dst.h },
		.uv = { q.src.x + q.src.w, q.src.y + q.src.h },
		.color = q.c2, 
		.size = { q.dst.w, q.dst.h },
		.radius = q.radius,
		.rect_pos = { q.dst.w, q.dst.h },
	};
	Vertex v5 = { 
		.pos = { q.dst.x, q.dst.y + q.dst.h },
		.uv = { q.src.x, q.src.y + q.src.h },
		.color = q.c3, 
		.size = { q.dst.w, q.dst.h },
		.radius = q.radius,
		.rect_pos = { 0, q.dst.h },
	};
	Vertex v6 = { 
		.pos = { q.dst.x, q.dst.y },
		.uv = { q.src.x, q.src.y },
		.color = q.c0, 
		.size = { q.dst.w, q.dst.h },
		.radius = q.radius,
		.rect_pos = { 0, 0 },
	};
	push_vertex(v1);
	push_vertex(v2);
	push_vertex(v3);
	push_vertex(v4);
	push_vertex(v5);
	push_vertex(v6);
}

/* -------------------------------------------------------------------------- */
/*                                 public api                                 */
/* -------------------------------------------------------------------------- */

void
render_init(void)
{
	init_font_atlas();
	init_shader_program();
	init_vao();
}

void
start_drawing(int window_width, int window_height)
{
	glViewport(0, 0, window_width, window_height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glUseProgram(program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font_atlas.texture);
	glUniform1i(glGetUniformLocation(program, "texture1"), 0);
	glUniform2f(glGetUniformLocation(program, "u_resolution"), window_width, window_height);
}

void
draw_rect_gradient(float x, float y, float w, float h, Color c0, Color c1, Color c2, Color c3)
{
	QuadData q = {
		.src = {
			(font_atlas.white_pixel.x * 1.0f) / font_atlas.w,
			(font_atlas.white_pixel.y * 1.0f) / font_atlas.h,
			0,
			0,
		},
		.dst = { x, y, w, h },
		.c0 = c0,
		.c1 = c1,
		.c2 = c2,
		.c3 = c3,
		.radius = 20.0f, // probably should be passed as a parameter
	};
	draw_quad(q);
}


void
draw_rect(float x, float y, float w, float h, Color color)
{
	draw_rect_gradient(x, y, w, h, color, color, color, color);
}

void
draw_triangle(float x0, float y0, float x1, float y1, float x2, float y2, Color color)
{
        push_simple_vertex(x0, y0, color);
        push_simple_vertex(x1, y1, color);
        push_simple_vertex(x2, y2, color);
}

void
draw_circle(float x, float y, float radius, Color color)
{
	QuadData q = {
		.src = {
			(font_atlas.white_pixel.x * 1.0f) / font_atlas.w,
			(font_atlas.white_pixel.y * 1.0f) / font_atlas.h,
			0,
			0,
		},
		.dst = { x - radius, y - radius, radius * 2, radius * 2 },
		.c0 = color,
		.c1 = color,
		.c2 = color,
		.c3 = color,
		.radius = radius,
	};
	draw_quad(q);
}

void
draw_line(float x0, float y0, float x1, float y1, float width, Color color)
{
        Vec2 dir = {x1 - x0, y1 - y0};
        Vec2 norm = vec2_norm(dir);
        Vec2 off0 = vec2_rotate(norm, PI / 2.0f);
        Vec2 off1 = vec2_rotate(norm, -PI / 2.0f);
        Vec2 p0 = {x0 + off0.x * width, y0 + off0.y * width};
        Vec2 p1 = {x0 + off1.x * width, y0 + off1.y * width};
        Vec2 p2 = {x1 + off0.x * width, y1 + off0.y * width};
        Vec2 p3 = {x1 + off1.x * width, y1 + off1.y * width};
        push_simple_vertex(p0.x, p0.y, color);
        push_simple_vertex(p1.x, p1.y, color);
        push_simple_vertex(p2.x, p2.y, color);
        push_simple_vertex(p1.x, p1.y, color);
        push_simple_vertex(p2.x, p2.y, color);
        push_simple_vertex(p3.x, p3.y, color);
}

void
finish_drawing(void)
{
	flush_vertices();
}
