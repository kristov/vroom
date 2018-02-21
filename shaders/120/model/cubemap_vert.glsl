#version 120

attribute vec3 b_vertex;

uniform mat4 m_mvp;

varying vec3 v_pos;

void main(void) {
    v_pos = normalize(b_vertex);
    gl_Position = m_mvp * vec4(b_vertex, 1.0);
}
