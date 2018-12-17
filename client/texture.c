#include <stdio.h>
#include <string.h>

#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// destination == where the square should go
// source    == source data
// square_px == nr. pixels per side of the square
// bytes_pl  == bytes per line original image
// bytes_pp  == bytes per pixel
// x_idx     == index of the square in x
// y_idx     == index of the square in y
// dest_off  == location to put the data
void texture_copy_square(uint8_t* destination, uint8_t* source, uint32_t square_px, uint32_t bytes_pl, uint32_t bytes_pp, uint32_t x_idx, uint32_t y_idx, uint32_t dest_off) {
    uint32_t i, start, length;

    start = ((square_px * y_idx) * bytes_pl) + ((square_px * x_idx) * bytes_pp);
    length = (square_px * bytes_pp);

    for (i = 0; i < square_px; i++) {
        memcpy(&destination[dest_off], &source[start], length);
        start += bytes_pl;
        dest_off += length;
    }
}

void texture_init(texture_t* texture, const char* filename) {
    texture->data = stbi_load(filename, &texture->width, &texture->height, &texture->bytes_per_pixel, 3);
    if (!texture->data) {
        return;
    }
}

void texture_cubemap_init(texture_cubemap_t* cubemap, const char* filename) {
    texture_t* texture = &cubemap->texture;
    texture_init(texture, filename);

    uint32_t square_width = texture->width / 4;
    cubemap->square_width = square_width;

    cubemap->bytes_per_line = texture->width * texture->bytes_per_pixel;
    cubemap->bytes_per_square = square_width * square_width * texture->bytes_per_pixel;
    cubemap->total_size = cubemap->bytes_per_square * 6;
}

void texture_cubemap_build_data(texture_cubemap_t* cubemap, uint8_t* destination) {
    uint32_t dest_off = 0;
    uint8_t* data = cubemap->texture.data;

    uint32_t square_px = cubemap->square_width;
    uint32_t bytes_pl = cubemap->bytes_per_line;
    uint32_t bytes_pp = cubemap->texture.bytes_per_pixel;
    uint32_t bytes_sq = cubemap->bytes_per_square;

    //       [YPOS]
    // [XNEG][ZPOS][XPOS][ZNEG]
    //       [YNEG]

    // XPOS
    texture_copy_square(destination, data, square_px, bytes_pl, bytes_pp, 2, 1, dest_off);
    dest_off += bytes_sq;

    // XNEG
    texture_copy_square(destination, data, square_px, bytes_pl, bytes_pp, 0, 1, dest_off);
    dest_off += bytes_sq;

    // YPOS
    texture_copy_square(destination, data, square_px, bytes_pl, bytes_pp, 1, 2, dest_off);
    dest_off += bytes_sq;

    // YNEG
    texture_copy_square(destination, data, square_px, bytes_pl, bytes_pp, 1, 0, dest_off);
    dest_off += bytes_sq;

    // ZPOS
    texture_copy_square(destination, data, square_px, bytes_pl, bytes_pp, 1, 1, dest_off);
    dest_off += bytes_sq;

    // ZNEG
    texture_copy_square(destination, data, square_px, bytes_pl, bytes_pp, 3, 1, dest_off);
    dest_off += bytes_sq;
}
