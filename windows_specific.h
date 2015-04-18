#ifdef WIN32
#include <windows.h> //for screen capturing
#ifndef WINDOWS_SPECIFIC_H
#define WINDOWS_SPECIFIC_H


class Win
{
    public:
    inline int GetFilePointer(HANDLE FileHandle);
    bool saveBMPFile(char *filename, HBITMAP bitmap, HDC bitmapDC, int width, int height);
    void screenCapture(std::string filename);
//        windows_specific();
//        virtual ~windows_specific();
    protected:
    private:
};

#endif // WINDOWS_SPECIFIC_H
#endif
