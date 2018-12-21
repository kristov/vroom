#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "memory_layout.h"
#include "vroom.h"

void geometry_plane_color(memory_layout_t* layout, float xmin, float ymin, float xmax, float ymax, float r, float g, float b, float a);

void geometry_cube_color(memory_layout_t* layout, float x, float y, float z, float r, float g, float b, float a);

#endif
