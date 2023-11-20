#version 130

uniform vec2 quad_size;
uniform vec2 quad_center;
uniform int hot;
uniform int widgetActive;
uniform int bevel;
uniform int border;
uniform int uBackground;
uniform vec4 uBoxColor;

in vec2 world_pos;
in vec2 tex_coord;
out vec4 color;

void main() {
	vec4 box = uBoxColor;
	vec4 boxHot = vec4(uBoxColor.xyz*1.2f, uBoxColor.a);
	vec4 boxLight = vec4(uBoxColor.xyz*1.5f, uBoxColor.a);
	vec4 boxDark = vec4(uBoxColor.xyz*0.7f, uBoxColor.a);
	vec4 boxShadow = vec4(uBoxColor.xyz*0.1f, uBoxColor.a);
	vec4 result = uBoxColor;
	float a = 1.0f;
	
	if(uBackground==0) {
		//discard;
		//a = 0.0f;
		result.a = 0.0f;
	}
	
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
	
	if(hot==1 && uBackground>0) {
		result = boxHot;
	}
	
	if(bevel==1) {
		if(widgetActive==1) {
			if(diff.x > (quad_size.x/2 - 0.22f) ||
			   diff.y > (quad_size.y/2 - 0.22f) ||
			   diff.x < -(quad_size.x/2 - 0.22f) ||
			   diff.y < -(quad_size.y/2 - 0.22f)) {
				result = boxDark;
				if(diff.x > (quad_size.x/2 - 0.1f) ||
				   diff.y > (quad_size.y/2 - 0.1f) ||
				   diff.x < -(quad_size.x/2 - 0.1f) ||
				   diff.y < -(quad_size.y/2 - 0.1f)) {
					result = boxShadow;
				}
			}
		} else {
			if(diff.x > (quad_size.x/2 - 0.22f) ||
			   diff.y > (quad_size.y/2 - 0.22f)) {
				result = boxDark;
			}
			
			if(diff.x < -(quad_size.x/2 - 0.1f) ||
			   diff.y < -(quad_size.y/2 - 0.1f)) {
				result = boxLight;
			}
			
			if(diff.x > (quad_size.x/2 - 0.1f) ||
			   diff.y > (quad_size.y/2 - 0.1f)) {
				result = boxShadow;
			}
		}
	}
	
	if(border>0) {
		if(diff.x > (quad_size.x/2 - 0.22f) ||
		   diff.y > (quad_size.y/2 - 0.22f) ||
		   diff.x < -(quad_size.x/2 - 0.22f) ||
		   diff.y < -(quad_size.y/2 - 0.22f)) {
			result = boxShadow;
		}
	}
	
	
	color = result;
	//gl_FragColor = vec4(1, 0, 0, 1);
}