main:
	gcc -g3 -O0 -I/d/software/dev/applib/cairo/include main.c `pkg-config --cflags --libs --static cairo-win32` -lgdi32 -o test.exe -std=gnu99

