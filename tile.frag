#version 130

uniform sampler2D texture0;
uniform float time;
uniform vec2 camera;
uniform vec2 scale;

// varying in vec2 screen_pos;
// varying in vec2 world_pos;
in vec2 tex_coord;
out vec4 color;

void main() {
	color = texture(texture0, tex_coord);
	//gl_FragColor = vec4(1, 0, 0, 1);
}