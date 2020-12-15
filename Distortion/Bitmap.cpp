#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }
   }
   free(pImageCodecInfo);
   return -1;  // Failure
}

int SaveBitmap(HBITMAP hBitmap, char *FileName)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    WCHAR FileNameW[MAX_PATH+1];

    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    mbstowcs(FileNameW, FileName, MAX_PATH+1);
    //HBITMAP hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), "babe.bmp", IMAGE_BITMAP, 0,0, LR_LOADFROMFILE);
    Bitmap *image = new Bitmap(hBitmap, NULL);

    CLSID myClsId;
    int retVal = GetEncoderClsid(L"image/png", &myClsId);

    image->Save(FileNameW, &myClsId, NULL);
    delete image;

    GdiplusShutdown(gdiplusToken);
    return 0;
}
