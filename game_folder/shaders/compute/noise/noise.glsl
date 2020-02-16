#version 430
//4*4 ray bundle
#define group_size 8
#define buffer_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba8, binding = 0) uniform image2D final_color; //final output texture 1 (used as final color)
//global, always the last ones
layout(rgba32f, binding = 1) uniform image2D DE_input; //global tex
layout(rgba32f, binding = 2) uniform image2D color_HDR; //global tex

#define PI 3.14159265
#include<utility/RNG.glsl>

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	vec2 img_size = vec2(imageSize(color_HDR));
	
	vec2 uv = vec2(global_pos)/img_size;
	
	//vec3 col = 0.5*perlin4(25*uv).xyz + 0.5;
	vec3 col = blue3(300.*uv, float(iFrame)*0.2);
	imageStore(final_color, global_pos, vec4(col, 1));	 
}