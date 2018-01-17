#version 120

attribute vec3 b_vertex;
attribute vec3 b_normal;
attribute vec2 b_uv;

uniform mat4 m_mvp;

varying vec3 v_vertex;
varying vec3 v_normal;
varying vec2 v_uv;

void main(void) {
    v_vertex = b_vertex;
    v_normal = b_normal;
    v_uv = b_uv;
    gl_Position = m_mvp * vec4(b_vertex, 1.0);
}
