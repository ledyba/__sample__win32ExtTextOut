#include <Windows.h>
#include <cairo/cairo.h>

int main(int argc, char** argv)
{
	cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 320, 320);
	cairo_t* cairo = cairo_create(surf);
	HDC hdc = CreateCompatibleDC(NULL);
	HBITMAP bmp = CreateCompatibleBitmap (hdc, 320, 320);
	SelectObject(hdc, bmp);
	ExtTextOut(hdc, 0,0,0,NULL, "test",4,NULL);
	
	DeleteObject(bmp);
	DeleteDC(hdc);
	cairo_destroy(cairo);
	cairo_surface_destroy(surf);
	return 0;
}

