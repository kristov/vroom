#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "memory_layout.h"
#include "vroom.h"

void geometry_plane_color(memory_layout_t* layout, float xmin, float ymin, float xmax, float ymax, float r, float g, float b, float a);

void geometry_cube_color(memory_layout_t* layout, float x, float y, float z, float r, float g, float b, float a);

void geometry_plane_generate_verticies(float* verts, float x_min, float y_min, float x_max, float y_max);

void geometry_plane_generate_normals(float* norms);

void geometry_plane_generate_indicies(uint16_t* indicies);

void geometry_plane_generate_uvs(float* uvs);

#endif
