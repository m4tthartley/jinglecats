#version 130

uniform sampler2D texture0;
uniform float time;
uniform vec2 camera;
uniform vec2 scale;

in vec2 screen_pos;
in vec2 world_pos;

// from Inigo Quilez
float hash(float n) {
	return fract(sin(n)*753.5453123);
}
float noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix(hash(n+0.0), hash(n+1.0), f.x), mix(hash(n+157.0), hash(n+158.0), f.x), f.y),
               mix(mix(hash(n+113.0), hash(n+114.0), f.x), mix(hash(n+270.0), hash(n+271.0), f.x), f.y), f.z);
}
float fbm(vec3 x) {
    float a = noise(x);
    a += noise(x * 2.0) / 2.0;
    a += noise(x * 4.0) / 4.0;
    a += noise(x * 8.0) / 8.0;
    a += noise(x * 16.0) / 16.0;
    return a;
}

void main() {
	vec3 sun = normalize(vec3(/*sin(time), cos(time)*/1.0, 1.0, 1.0));
	//float spx = floor((screen_pos.x+(camera.x*scale.x)+(time*0.01))*(1920.0/8))*0.05;
	float spx = floor(world_pos.x*(32))*0.02;
	float spy = floor(world_pos.y*(32))*0.02;
	vec2 sp = vec2(spx,spy);
	float f = fbm(vec3(sp*2, 0));
	//vec3 normal = normalize(vec3(
	//	fbm(vec3(sp*10, 0))*2-1,
	//	fbm(vec3(sp*10, 10)),
	//	fbm(vec3(sp*10, 20))*2-1
	//));

	/*vec3 p1 = vec3(sp+vec2(-0.05,-0.05), 0);
	vec3 p2 = vec3(sp+vec2( 0.05,-0.05), 0);
	vec3 p3 = vec3(sp+vec2( 0.05, 0.05), 0);
	vec3 p4 = vec3(sp+vec2(-0.05, 0.05), 0);
	p1.z = fbm(p1*4);
	p2.z = fbm(p2*4);
	p3.z = fbm(p3*4);
	p4.z = fbm(p4*4);*/
	/*vec2 sp1 = sp+vec2(-0.005,-0.005);
	vec2 sp2 = sp+vec2( 0.005,-0.005);
	vec2 sp3 = sp+vec2( 0.005, 0.005);
	vec2 sp4 = sp+vec2(-0.005, 0.005);*/
	/*vec3 p1 = vec3(sp1, min(1-abs(sp1.x), 1-abs(sp1.y)));
	vec3 p2 = vec3(sp2, min(1-abs(sp2.x), 1-abs(sp2.y)));
	vec3 p3 = vec3(sp3, min(1-abs(sp3.x), 1-abs(sp3.y)));
	vec3 p4 = vec3(sp4, min(1-abs(sp4.x), 1-abs(sp4.y)));*/
	/*vec3 p1 = vec3(sp1, fbm(vec3(sp1*3, 0))+0.0);
	vec3 p2 = vec3(sp2, fbm(vec3(sp2*3, 0))+0.0);
	vec3 p3 = vec3(sp3, fbm(vec3(sp3*3, 0))+0.0);
	vec3 p4 = vec3(sp4, fbm(vec3(sp4*3, 0))+0.0);*/
	vec2 sp1 = sp+vec2(  -0.01,0);
	vec2 sp2 = sp+vec2(   0.01,0);
	vec2 sp3 = sp+vec2(0,-0.01);
	vec2 sp4 = sp+vec2(0, 0.01);
	float h1 = fbm(vec3(sp1*2, 0));
	float h2 = fbm(vec3(sp2*2, 0));
	float h3 = fbm(vec3(sp3*2, 0));
	float h4 = fbm(vec3(sp4*2, 0));

	vec3 normal; //= (cross(normalize(p2-p1), normalize(p3-p1)));
	normal.x = h1-h2;
	normal.y = h3-h4;
	normal.z = 0.5;
	normal = normalize(normal);
	//gl_FragColor = vec4(vec3(1)*(f*0.5+0.5), 1);
	//gl_FragColor = vec4(sin(time*10), 0,0, 1);
	float diff = dot(normal, sun);
	vec3 normalLight = vec3(diff*0.2+0.85);
	float sunDirLight = dot(normalize(sp), sun.xy);
	gl_FragColor = vec4(/*vec3(0.5)+vec3(1,1,1)*diff*0.5*/ /*vec3(p1.xy*0.5+0.5, p1.z)*/ 
	/*mix(normalLight, vec3(1.0, 0.2, 0.2), sunDirLight)*/
	pow(mix(normalLight, /*vec3(1.0, 0.5, 0.0)**/normalLight, max(sunDirLight, 0)), vec3(5)), 1);
	//gl_FragColor.r = world_pos.x;
	//if(noise(vec3((world_pos+camera)*100,0)) > 0.98) {
	//	gl_FragColor = vec4(1);
	//}
}