#version 100

precision mediump int;
precision mediump float;

attribute vec3 b_vertex;
attribute vec3 b_normal;

uniform mat4 m_mvp;
uniform mat4 m_mov;
uniform vec4 u_color;

varying vec4 v_color;
varying vec3 v_normal;
varying vec3 v_vertex;

void main(void) {
    v_color = u_color;
    v_normal = vec3(m_mov * vec4(b_normal, 0.0)); // m_mov is bad
    v_vertex = vec3(m_mov * vec4(b_vertex, 0.0)); // m_mov is bad
    gl_Position = m_mvp * vec4(b_vertex, 1.0);
}
