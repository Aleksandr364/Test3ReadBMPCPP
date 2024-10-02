#include "Struct.h"

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "Format: drawBmp.exe image_file_path\n";
    return 0;
  }
  BMPFile TestObj(argv[1]);
}