#version 100

precision mediump int;
precision mediump float;

varying vec4 v_color;
varying vec3 v_normal;
varying vec3 v_vertex;

void main(void) {
    vec3 u_light = vec3(0.0, 0.0, 0.0);
    float distance = length(u_light - v_vertex);
    vec3 light_vec = normalize(u_light - v_vertex);
    float diffuse = max(dot(v_normal, light_vec), 0.1);
    diffuse = diffuse * (1.0 / (1.0 + (0.25 * distance * distance)));
    gl_FragColor = v_color * diffuse;
}
