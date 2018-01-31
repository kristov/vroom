#version 120

uniform mat4 m_mv;

varying vec3 v_vertex;
varying vec3 v_normal;

uniform sampler2D s_tex;
varying vec2 v_uv;

void main(void) {
    vec3 normal_ms = normalize(vec3(m_mv * vec4(v_normal, 0.0)));
    vec3 light_ms = vec3(0.0, 0.0, 0.0);
    vec3 vert_ms = vec3(m_mv * vec4(v_vertex, 1.0));
    vec3 stl = light_ms - vert_ms;

    float brightness = dot(normal_ms, stl) / (length(stl) * length(normal_ms));
    brightness = clamp(brightness, 0.0, 1.0);

    gl_FragColor = texture2D(s_tex, v_uv) * brightness;
}
