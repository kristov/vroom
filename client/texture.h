#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>

typedef struct texture {
    int32_t width;
    int32_t height;
    int32_t bytes_per_pixel;
    uint32_t total_size;
    uint8_t* data;
} texture_t;

typedef struct texture_cubemap {
    texture_t texture;
    uint32_t square_width;
    uint32_t bytes_per_line;
    uint32_t bytes_per_square;
    uint32_t total_size;
    uint8_t* data;
} texture_cubemap_t;

void texture_init(texture_t* texture, const char* filename);

void texture_cubemap_init(texture_cubemap_t* cubemap, const char* filename);

void texture_cubemap_build_data(texture_cubemap_t* cubemap, uint8_t* destination);

#endif
