#version 130


uniform vec2 quad_size;
uniform vec2 quad_center;

in vec2 world_pos;
in vec2 tex_coord;
out vec4 color;

void main() {
	vec2 diff = world_pos-quad_center;
	float d = length(diff);
	if(
	diff.x < -(quad_size.x/2 - 0.15f) ||
	diff.y > (quad_size.y/2 - 0.15f)
//abs(diff.x) > quad_size.x/2 - 0.15f || abs(diff.y) > quad_size.y/2 - 0.15f
) {
		color = vec4(1, 1, 1, 1);		
} else if(
	diff.x > (quad_size.x/2 - 0.15f) ||
	diff.y < -(quad_size.y/2 - 0.15f)
) {
		color = vec4(0, 0, 0, 1);	
	} else {
		color = vec4(1, 0, 0, 1);
	}
	//gl_FragColor = vec4(1, 0, 0, 1);
}