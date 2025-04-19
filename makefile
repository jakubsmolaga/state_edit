main: *.c *.h
	@cc -Wall -lm -lGLESv2 -lGLX -lX11 -o main *.c
