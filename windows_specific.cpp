#ifdef _WIN32
#include "windows_specific.h"

inline int GetFilePointer(HANDLE FileHandle){
	return SetFilePointer(FileHandle, 0, 0, FILE_CURRENT);
}

bool saveBMPFile(char *filename, HBITMAP bitmap, HDC bitmapDC, int width, int height){
	bool Success=0;
	HDC SurfDC=NULL;
	HBITMAP OffscrBmp=NULL;
	HDC OffscrDC=NULL;
	LPBITMAPINFO lpbi=NULL;
	LPVOID lpvBits=NULL;
	HANDLE BmpFile=INVALID_HANDLE_VALUE;
	BITMAPFILEHEADER bmfh;
	if ((OffscrBmp = CreateCompatibleBitmap(bitmapDC, width, height)) == NULL)		return 0;
	if ((OffscrDC = CreateCompatibleDC(bitmapDC)) == NULL)		return 0;
	HBITMAP OldBmp = (HBITMAP)SelectObject(OffscrDC, OffscrBmp);
	BitBlt(OffscrDC, 0, 0, width, height, bitmapDC, 0, 0, SRCCOPY);
	if ((lpbi = (LPBITMAPINFO)(new char[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)])) == NULL)
		return 0;
	ZeroMemory(&lpbi->bmiHeader, sizeof(BITMAPINFOHEADER));
	lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	SelectObject(OffscrDC, OldBmp);
	if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, NULL, lpbi, DIB_RGB_COLORS))		return 0;
	if ((lpvBits = new char[lpbi->bmiHeader.biSizeImage]) == NULL)		return 0;
	if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, lpvBits, lpbi, DIB_RGB_COLORS))		return 0;
	if ((BmpFile = CreateFile(filename,	GENERIC_WRITE,
						0, NULL,
						CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,
						NULL)) == INVALID_HANDLE_VALUE)
		return 0;
	DWORD Written;
	bmfh.bfType = 19778;
	bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
	if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))		return 0;
	if (Written < sizeof(bmfh))		return 0;
	if (!WriteFile(BmpFile, &lpbi->bmiHeader, sizeof(BITMAPINFOHEADER), &Written, NULL))		return 0;
	if (Written < sizeof(BITMAPINFOHEADER))		return 0;
	int PalEntries;
	if (lpbi->bmiHeader.biCompression == BI_BITFIELDS)		PalEntries = 3;
	else PalEntries = (lpbi->bmiHeader.biBitCount <= 8) ?  (int)(1 << lpbi->bmiHeader.biBitCount) : 0;
	if(lpbi->bmiHeader.biClrUsed)	PalEntries = lpbi->bmiHeader.biClrUsed;
	if(PalEntries){
	if (!WriteFile(BmpFile, &lpbi->bmiColors, PalEntries * sizeof(RGBQUAD), &Written, NULL))
		return 0;
		if (Written < PalEntries * sizeof(RGBQUAD))			return 0;
	}
	bmfh.bfOffBits = GetFilePointer(BmpFile);
	if (!WriteFile(BmpFile, lpvBits, lpbi->bmiHeader.biSizeImage, &Written, NULL))
		return 0;
	if (Written < lpbi->bmiHeader.biSizeImage)
		return 0;
	bmfh.bfSize = GetFilePointer(BmpFile);
	SetFilePointer(BmpFile, 0, 0, FILE_BEGIN);
	if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))		return 0;
	if (Written < sizeof(bmfh))		return 0;
    //http://msdn.microsoft.com/en-us/library/aa365715%28v=vs.85%29.aspx
    UnlockFile(BmpFile,0, sizeof(bmfh), 0, sizeof(Written));
	return 1;
}

void screenCapture(std::string filename){
    //int w = 512; int h = 512;
    int w = GetSystemMetrics(SM_CXSCREEN); int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hDc = CreateCompatibleDC(0);
    HBITMAP hBmp = CreateCompatibleBitmap(GetDC(0), w, h);
    SelectObject(hDc, hBmp);
    BitBlt(hDc, 0, 0, w, h, GetDC(0), 0, 0, SRCCOPY);
    char* fname = new char[filename.size() + 1];
    std::copy(filename.begin(), filename.end(), fname);
    fname[filename.size()] = '\0'; //terminating null character
    SaveBMPFile(fname, hBmp, hDc, w, h);
    DeleteObject(hBmp);

}
#endif //end of windows specific methods
