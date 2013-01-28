#include <Windows.h>
#include <cairo/cairo.h>
#include <stdio.h>
#include <memory.h>

int main(int argc, char** argv)
{
	const int width = 256;
	const int height = 256;
	unsigned char* data = malloc(width*height*3);
	memset(data, 0xff, width*height*3);
	if( data == NULL ){
		fprintf(stderr, "Oops. failed to create buffer.");
		return -1;
	}
	{
		HDC hdc = CreateCompatibleDC(NULL);
		if( hdc == NULL ) {
			fprintf(stderr, "Oops. failed to create compatible dc.");
			return -1;
		}
		HBITMAP bmp;
		BITMAPINFOHEADER ih;
		{
			memset(&ih, 0, sizeof(BITMAPINFOHEADER));
			ih.biSize = sizeof(BITMAPINFOHEADER);
			ih.biWidth = width;
			ih.biHeight = height;
			ih.biPlanes = 1;
			ih.biBitCount = 24;
			ih.biCompression = BI_RGB;
			ih.biSizeImage  = 0;
			ih.biXPelsPerMeter = 0;
			ih.biYPelsPerMeter = 0;
			ih.biClrUsed = 0;
			ih.biClrImportant = 0;
			bmp = CreateDIBitmap(hdc, &ih, CBM_INIT, data, (BITMAPINFO*)&ih, DIB_RGB_COLORS);
			if( bmp == NULL ) {
				fprintf(stderr, "Oops. failed to create bitmap.");
				return -1;
			}
			if( SelectObject(hdc, bmp) == 0 ) {
				fprintf(stderr, "Oops. failed to select object.");
				return -1;
			}
		}
		HFONT font;
		{
			LOGFONT logfont;
			memset (&logfont, 0, sizeof (logfont));

			// Create a GDI Times New Roman font.
			logfont.lfHeight = 18;
			logfont.lfWidth = 0;
			logfont.lfEscapement = 0;
			logfont.lfOrientation = 0;
			logfont.lfWeight = FW_BOLD;
			logfont.lfItalic = FALSE;
			logfont.lfUnderline = FALSE;
			logfont.lfStrikeOut = FALSE;
			logfont.lfCharSet = DEFAULT_CHARSET;
			logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
			logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			logfont.lfQuality = DEFAULT_QUALITY;
			logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
			memcpy (logfont.lfFaceName, "Times New Roman", LF_FACESIZE);
			font = CreateFontIndirect (&logfont);
			if( font == NULL) {
				fprintf(stderr, "Oops. failed to create font.");
				return -1;
			}
			if( SelectObject(hdc, font) == 0 ) {
				fprintf(stderr, "Oops. failed to select font object.");
				return -1;
			}
		}
		if( SetBkColor(hdc, RGB(0,255,255)) == CLR_INVALID ) {
			fprintf(stderr, "Oops. failed to set text color.");
			return -1;
		}
		if( SetTextColor(hdc, RGB(255,0,0)) == CLR_INVALID ) {
			fprintf(stderr, "Oops. failed to set text color.");
			return -1;
		}
		if( SetBkMode(hdc,TRANSPARENT) == 0 ) {
			fprintf(stderr, "Oops. failed to set mode.");
			return -1;
		}
		if( ExtTextOut(hdc, 128,128, 0, NULL, "test", 4, NULL) == 0 ) {
			fprintf(stderr, "Oops. failed to set text color.");
			return -1;
		}
		GetDIBits(
				hdc,           // デバイスコンテキストのハンドル
				bmp,      // ビットマップのハンドル
				0,   // 取得対象の最初の走査行
				256,   // 取得対象の走査行の数
				data,    // ビットマップのビットからなる配列
				(BITMAPINFO*)&ih, // ビットマップデータのバッファ
				DIB_RGB_COLORS// RGB とパレットインデックスのどちらか
				);

		DeleteObject(bmp);
		DeleteObject(font);
		DeleteDC(hdc);
	}
	
	//strideの問題があるのでコピー
	unsigned char* cdata = malloc(width*height*4);
	for(int y=0;y<height;++y){
		for(int x=0;x<width;++x){
			((unsigned int*)cdata)[y*width + x] = *((unsigned int*)&data[(y*width+x)*3]) & 0xffffff;
		}
	}
	cairo_surface_t* surf = cairo_image_surface_create_for_data(cdata, CAIRO_FORMAT_RGB24, width, height, width*4);
	if(cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS){
		fprintf(stderr, "Oops. Invalid status: %s", cairo_status_to_string(cairo_surface_status(surf)));
		return -1;
	}
	cairo_surface_write_to_png(surf, "test.png");
	cairo_surface_destroy(surf);
	free(data);
	free(cdata);
	return 0;
}

