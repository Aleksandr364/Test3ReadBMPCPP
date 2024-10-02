#pragma once

#include <windows.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <iostream>

#pragma pack(push) // Сохранить старое значение выравнивания.
#pragma pack(1) // Требует выравнивание на 1 байт.

struct sBITMAPFILEHEADER
{
  WORD  bfType = 0x4d42; // Сигнатура BMP-файла = "BM", если нет, значит, файл - не BMP.
  DWORD bfSize = 0; // The size, in bytes, of the bitmap file.
  WORD  bfReserved1 = 0; // Reserved; must be zero.
  WORD  bfReserved2 = 0; // Reserved; must be zero.
  DWORD bfOffBits = 0; // The offset, in bytes, from the beginning of the BITMAPFILEHEADER structure to the bitmap bits.
};

struct sBITMAPINFOHEADER
{
  DWORD biSize = 0; // Размер структуры в байтах.
  LONG  biWidth = 0; // The width of the bitmap, in pixels.
  LONG  biHeight = 0; // The height of the bitmap, in pixels.
  // For uncompressed RGB bitmaps, if biHeight is positive, the bitmap is a bottom - up DIB with the origin at the lower left corner.If biHeight is negative, the bitmap is a top - down DIB with the origin at the upper left corner.
  // For YUV bitmaps, the bitmap is always top - down, regardless of the sign of biHeight.Decoders should offer YUV formats with positive biHeight, but for backward compatibility they should accept YUV formats with either positive or negative biHeight.
  // For compressed formats, biHeight must be positive, regardless of image orientation.
  WORD  biPlanes = 1; // The number of planes for the target device. This value must be set to 1.
  WORD  biBitCount = 0; // The number of bits per pixel (bpp).
  // For uncompressed formats, this value is the average number of bits per pixel.
  // For compressed formats, this value is the implied bit depth of the uncompressed image, after the image has been decoded.
  DWORD biCompression = BI_RGB; // Uncompressed RGB.
  DWORD biSizeImage = 0; // The size, in bytes, of the image. This can be set to 0 for uncompressed RGB bitmaps.
  LONG  biXPelsPerMeter = 0; // The horizontal resolution, in pixels per meter, of the target device for the bitmap.
  LONG  biYPelsPerMeter = 0; // The vertical resolution, in pixels per meter, of the target device for the bitmap.
  DWORD biClrUsed = 0; // The number of color indices in the color table by the bitmap.
  DWORD biClrImportant = 0; // The number of color indices that are considered important for displaying the bitmap.
};

struct sRGB
{
  BYTE red = 0;
  BYTE green = 0;
  BYTE blue = 0;
};

#pragma pack(pop) // Восстанавливает значение выравнивания по умолчанию.

class BMPFile
{
protected:

  char* data = nullptr;
  size_t fileSize = 0;
  sBITMAPFILEHEADER* fHeader = nullptr;
  sBITMAPINFOHEADER* iHeader = nullptr;
  std::string fileName;
  size_t lenBYTE = 0;
  size_t widthBYTE = 0; // Число бай в строке с учётом "гэпа".
  size_t gap = 0;

  bool openBMP(const std::string& fileName)
  {
    std::ifstream file(fileName, std::ios::binary);
    if (!file.is_open()) return false;
    fileSize = std::filesystem::file_size(fileName);
    data = new char[fileSize] {};
    file.read(data, fileSize);
    file.close();
    fHeader = (sBITMAPFILEHEADER*)data;
    if (fHeader->bfType != 0x4d42) return false;
    iHeader = (sBITMAPINFOHEADER*)(data + sizeof(sBITMAPFILEHEADER));
    if (iHeader->biBitCount != 24 && iHeader->biBitCount != 32) return false;
    lenBYTE = iHeader->biBitCount / 8;
    widthBYTE = iHeader->biWidth * lenBYTE;
    size_t work = (widthBYTE / 4) * 4;
    if (work != widthBYTE)
      gap = work + 4 - widthBYTE;
    widthBYTE += gap;
    size_t fileSizeExp = fHeader->bfOffBits + iHeader->biHeight * widthBYTE;
    if (fileSizeExp > fileSize) return false;
    return true;
  }

  void displayBMPNeg()
  {
    BYTE* cData = (BYTE*)(data + fHeader->bfOffBits);
    for (size_t i = 0; i < std::abs(iHeader->biHeight); i++)
    {
      for (size_t j = 0; j < iHeader->biWidth; j++)
      {
        if (cData[0] == 0 && cData[1] == 0 && cData[2] == 0)
          std::cout << "+";
        else if (cData[0] == 255 && cData[1] == 255 && cData[2] == 255)
          std::cout << "-";
        //else if (cData[0] == 255 && cData[1] == 0 && cData[2] == 255)
        //  std::cout << "*";
        //else if (cData[0] == 8 && cData[1] == 8 && cData[2] == 8)
        //  std::cout << "=";
        else
          std::cout << " ";
        cData += lenBYTE;
      }
      cData += gap;
      std::cout << "\n";
    }
  }

  void displayBMP2Pos()
  {
    BYTE* cDataBeg = (BYTE*)(data + fHeader->bfOffBits);
    for (int i = iHeader->biHeight - 1; i >= 0; i--)
    {
      BYTE* cData = cDataBeg + i * widthBYTE;
      for (size_t j = 0; j < iHeader->biWidth; j++)
      {
        if (cData[0] == 0 && cData[1] == 0 && cData[2] == 0)
          std::cout << "+";
        else if (cData[0] == 255 && cData[1] == 255 && cData[2] == 255)
          std::cout << "-";
        //else if (cData[0] == 255 && cData[1] == 0 && cData[2] == 255)
        //  std::cout << "*";
        //else if (cData[0] == 8 && cData[1] == 8 && cData[2] == 8)
        //  std::cout << "=";
        else
          std::cout << " ";
        cData += lenBYTE;
      }
      std::cout << "\n";
    }
  }

  void displayBMP()
  {
    if (iHeader->biHeight > 0)
      displayBMP2Pos();
    else
      displayBMPNeg();
  }

  void closeBMP()
  {
    delete[] data;
    data = nullptr;
  }

public:

  BMPFile(const std::string& fileName) : fileName(fileName)
  {
    openBMP(fileName);
    displayBMP();
  }
  ~BMPFile() { closeBMP(); }
};