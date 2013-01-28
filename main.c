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
		// cairoにおけるcairo_t的な存在。これに対して描画コマンドを発行する
		HDC hdc = CreateCompatibleDC(NULL);
		if( hdc == NULL ) {
			fprintf(stderr, "Oops. failed to create compatible dc.");
			return -1;
		}
		HBITMAP bmp;
		// .bmpのヘッダを書かないとRGBの生データを流し込めない（驚愕）
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
			// 前後のihの違いがよくわかんないんだけど、
			// 前者は初期データのフォーマットを、
			// 後者は作られるビットマップのフォーマットを
			// それぞれ示してるみたい。で、一緒のフォーマット（RGB24）なので同じ内容になる。
			// dataに初期データを入れてCBM_INITを指定するとそれが初期の絵になる。
			// ここではmemsetで0xffで指定したので真っ白。
			bmp = CreateDIBitmap(hdc, &ih, CBM_INIT, data, (BITMAPINFO*)&ih, DIB_RGB_COLORS);
			if( bmp == NULL ) {
				fprintf(stderr, "Oops. failed to create bitmap.");
				return -1;
			}
			// 描画先に指定。この関数何にでも使えちゃうので型安全性という概念はどこにもないのでは
			if( SelectObject(hdc, bmp) == 0 ) {
				fprintf(stderr, "Oops. failed to select object.");
				return -1;
			}
		}
		HFONT font;
		{
			// サンプルからとってきてフォントを作成
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
		// 背景色を指定してるけど、あとでトランスパレントを指定してるので多分意味ない
		if( SetBkColor(hdc, RGB(0,255,255)) == CLR_INVALID ) {
			fprintf(stderr, "Oops. failed to set text color.");
			return -1;
		}
		// XXX: テキストの色を指定してるんだけど反映されないです先生
		if( SetTextColor(hdc, RGB(255,255,0)) == CLR_INVALID ) {
			fprintf(stderr, "Oops. failed to set text color.");
			return -1;
		}
		// フォントの文字のある所以外に色を塗るかどうか指定。OPAQUEってやると矩形に色が塗られる（らしい）
		// ここでは透明なので、元の色が残る。
		if( SetBkMode(hdc,TRANSPARENT) == 0 ) {
			fprintf(stderr, "Oops. failed to set mode.");
			return -1;
		}
		// 文字出力
		if( ExtTextOut(hdc, 0,0, 0, NULL, "test", 4, NULL) == 0 ) {
			fprintf(stderr, "Oops. failed to set text color.");
			return -1;
		}
		
		// CreateDIBitmapすると新しいバッファが作られてそっちに描画されちゃうので、
		// この関数でそのバッファを読み出す
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
	// 上下反転してるのはbitmapの仕様
	// http://www.umekkii.jp/data/computer/file_format/bitmap.cgi
	// "通常画像データは左下から右上に記録されています。つまり、上下が反転しています。"
	unsigned char* cdata = malloc(width*height*4);
	for(int y=0;y<height;++y){
		for(int x=0;x<width;++x){
			((unsigned int*)cdata)[(height-y-1)*width + x] = *((unsigned int*)&data[(y*width+x)*3]) & 0xffffff;
		}
	}
	
	// cairoを使ってpngにコピー
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

