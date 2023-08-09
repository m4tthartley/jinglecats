#version 130

uniform vec2 quad_size;
uniform vec2 quad_center;
uniform int hot;
uniform int active;

in vec2 world_pos;
in vec2 tex_coord;
out vec4 color;

void main() {
	vec3 background = vec3(0.1f, 0.2f, 0.5f);
	vec3 result = background;
	
	vec2 diff = world_pos-quad_center;
	//float d = length(diff);
	//float right = quad_size.x/2 - (diff.x);
	//float top = quad_size.y/2 - (diff.y);
	//float left = -quad_size.x/2 - (diff.x);
	//float bottom = -quad_size.y/2 - (diff.y);
	
#if 0
	bool shadow = false;
	bool light = false;
	if(diff.x > (quad_size.x/2 - 0.22f) ||
	   diff.y > (quad_size.y/2 - 0.22f)) {
		if(diff.x > (quad_size.x/2 - 0.1f) ||
		   diff.y > (quad_size.y/2 - 0.1f)) {
			//result *= 0.1f;
		} else {
			//result *= 0.5f;
		}
		shadow = true;
	}
	if(diff.x > (quad_size.x/2 - 0.1f) ||
	   diff.y > (quad_size.y/2 - 0.1f)) {
		//result *= 1.5f;
		light = true;
	}
	
	vec2 p = world_pos-quad_center;
	vec2 l1 = vec2(p.x>0 ? quad_size.x/2-quad_size.y*2 : -quad_size.x/2, quad_size.y/2);
	vec2 l2 = vec2(p.x>0 ? quad_size.x/2 : -quad_size.x/2+quad_size.y/2, -quad_size.y/2);
	float line = (p.x-l1.x)*(l2.y-l1.y) - (p.y-l1.y)*(l2.x-l1.x);
	
	if(shadow && light) {
		if(line<0) {
			result *= 1.5f;			
		} else {
			if(diff.x > (quad_size.x/2 - 0.1f) ||
			   diff.y > (quad_size.y/2 - 0.1f)) {
				result *= 0.1f;
			} else {
				result *= 0.5f;
			}		
		}
	} else if(light){
		result *= 1.5f;
	} else if(shadow) {
		if(diff.x > (quad_size.x/2 - 0.1f) ||
		   diff.y > (quad_size.y/2 - 0.1f)) {
			result *= 0.1f;
		} else {
			result *= 0.5f;
		}
	}
#endif
	
	if(hot==1) {
		result = background * 1.2f;
	}
	
	if(active==1) {
		if(diff.x > (quad_size.x/2 - 0.22f) ||
		   diff.y > (quad_size.y/2 - 0.22f) ||
		   diff.x < -(quad_size.x/2 - 0.22f) ||
		   diff.y < -(quad_size.y/2 - 0.22f)) {
			result = background * 0.7f;
			if(diff.x > (quad_size.x/2 - 0.1f) ||
			   diff.y > (quad_size.y/2 - 0.1f) ||
			   diff.x < -(quad_size.x/2 - 0.1f) ||
			   diff.y < -(quad_size.y/2 - 0.1f)) {
				result = background * 0.1f;
			}
		}
	} else {
		if(diff.x > (quad_size.x/2 - 0.22f) ||
		   diff.y > (quad_size.y/2 - 0.22f)) {
			result = background * 0.7f;
		}
		
		if(diff.x < -(quad_size.x/2 - 0.1f) ||
		   diff.y < -(quad_size.y/2 - 0.1f)) {
			result = background * 1.5f;
		}
		
		if(diff.x > (quad_size.x/2 - 0.1f) ||
		   diff.y > (quad_size.y/2 - 0.1f)) {
			result = background * 0.1f;
		}
	}
	
	color = vec4(result, 1);
	//gl_FragColor = vec4(1, 0, 0, 1);
}