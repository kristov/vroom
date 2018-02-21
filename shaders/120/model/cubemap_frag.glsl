#version 120

uniform samplerCube s_tex;
varying vec3 v_pos;

void main(void) {
    gl_FragColor = textureCube(s_tex, v_pos);
}
