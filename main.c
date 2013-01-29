#include <Windows.h>
#include <cairo/cairo.h>
#include <stdio.h>
#include <memory.h>

int main(int argc, char** argv)
{
	const int width = 256;
	const int height = 256;
	unsigned char* data;
	unsigned char* cdata = malloc(width*height*4);
	{
		// cairoにおけるcairo_t的な存在。これに対して描画コマンドを発行する
		// この時、デフォルトの画像フォーマットは1と0の二値画像になってしまうっぽい（？）
		HDC hdc = CreateCompatibleDC(NULL);
		if( hdc == NULL ) {
			fprintf(stderr, "Oops. failed to create compatible dc.");
			return -1;
		}
		// .bmpのヘッダの構造体。これを書かないとRGBデータを殆ど扱えない（驚愕）
		// これで扱うフォーマットを指定。今回はRGB24ビット。
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
		}
		// cairoにおけるcairo_surface_t的な存在。
		HBITMAP bmp;
		{
			// bppやサイズを利用してビットマップを作成し、その出力先の生メモリも取得できる。
			// 最初CreateDIBitmap使ってたんだけど、これだとbppとかのフォーマットがCreateCompatibleDCで
			// 作ったDCに合わされて（？）1/0の二値画像になってしまって使い物にならなかった
			// 参考：http://eseken.dtiblog.com/blog-entry-74.html
			// "そもそも CreateCompatibleDC をして得られた device context は全く使えもしない 
			// 1 pixel * 1 pixel の白黒の bitmap などを持っている"
			bmp = CreateDIBSection(NULL, (BITMAPINFO*)&ih, DIB_RGB_COLORS, (void **)(&data), NULL, 0);
			if( bmp == NULL ) {
				fprintf(stderr, "Oops. failed to create bitmap.");
				return -1;
			}
			// HDCの実際に描画される先としてbitmapを指定。
			// この関数はブラシ（？）とかフォントとかにも使えるらしい。型安全性はたぶん無い。
			if( SelectObject(hdc, bmp) == 0 ) {
				fprintf(stderr, "Oops. failed to select object.");
				return -1;
			}
		}
		HFONT font;
		{
			// サンプルからとってきたフォントを作成するサンプルそのもの
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
		// テキストの色を設定
		if( SetTextColor(hdc, RGB(255,255,0)) == CLR_INVALID ) {
			fprintf(stderr, "Oops. failed to set text color.");
			return -1;
		}
		// 背景色を指定してるけど、下でトランスパレントを指定してるので多分意味ない
		if( SetBkColor(hdc, RGB(0,255,255)) == CLR_INVALID ) {
			fprintf(stderr, "Oops. failed to set text color.");
			return -1;
		}
		// フォントの文字のある所以外に色を塗るかどうか指定。OPAQUEってやると矩形に色が塗られる（らしい）
		// ここでは透明なので、元の色が残る。
		if( SetBkMode(hdc,TRANSPARENT) == 0 ) {
			fprintf(stderr, "Oops. failed to set mode.");
			return -1;
		}
		// 文字出力のためのメモリサイズはこうすると取得できるよ
		SIZE size;
		GetTextExtentPoint32(hdc, "test", 4, &size);
		fprintf(stdout, "size: %dx%d\n",size.cx, size.cy);
		// 文字出力
		if( ExtTextOut(hdc, 0,0, 0, NULL, "test", 4, NULL) == 0 ) {
			fprintf(stderr, "Oops. failed to set text color.");
			return -1;
		}
		
		{
			//CreateDIBSectionで取得できたchar*は、そのまま描画した後にコピーしてもよいらしい。
			
			// cairoのRGBはあくまでピクセルごとに４バイト(1バイト余る)なのでそれに合わせる
			// 上下反転してるのはbitmapの仕様
			// http://www.umekkii.jp/data/computer/file_format/bitmap.cgi
			// "通常画像データは左下から右上に記録されています。つまり、上下が反転しています。"
			for(int y=0;y<height;++y){
				for(int x=0;x<width;++x){
					((unsigned int*)cdata)[(height-y-1)*width + x] = *((unsigned int*)&data[(y*width+x)*3]) & 0xffffff;
				}
			}
		}
		DeleteObject(bmp);
		DeleteObject(font);
		DeleteDC(hdc);
	}
	
	// cairoを使ってpngに保存
	cairo_surface_t* surf = cairo_image_surface_create_for_data(cdata, CAIRO_FORMAT_RGB24, width, height, width*4);
	if(cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS){
		fprintf(stderr, "Oops. Invalid status: %s", cairo_status_to_string(cairo_surface_status(surf)));
		return -1;
	}
	cairo_surface_write_to_png(surf, "test.png");
	cairo_surface_destroy(surf);
	free(cdata);
	return 0;
}

