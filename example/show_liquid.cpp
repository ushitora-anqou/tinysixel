#include <iostream>

#include "../sixel.hpp"

// Use stb_image.h to load an image file.
// Thanks to: https://github.com/nothings/stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main()
{
    int w, h, n;
    uint8_t *pixels = stbi_load("liquid.png", &w, &h, &n, 4);
    assert(pixels != NULL && "Can't load the file!");

    Sixel sixel(std::cout);
    SixelImage sixelImg = SixelImage{w, h, pixels};
    sixel.print(sixelImg);

    return 0;
}
