#version 130

uniform vec2 world_position;
//uniform vec screen_size;
out vec2 screen_pos;
out vec2 world_pos;
out vec2 tex_coord;

void main() {
	screen_pos = (gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(gl_Vertex.xy+world_position, gl_Vertex.z, gl_Vertex.w)).xy;
	world_pos = gl_Vertex.xy+world_position;
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(gl_Vertex.xy+(world_position*1.0), gl_Vertex.z, gl_Vertex.w);
	gl_TexCoord[0] = gl_MultiTexCoord0;
	tex_coord = gl_MultiTexCoord0.xy;
}