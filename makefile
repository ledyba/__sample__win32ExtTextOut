main:
	gcc -g -O0 -I/d/software/dev/applib/cairo/include main.c `pkg-config --cflags --libs --static cairo-win32` -lgdi32 -o test.exe

