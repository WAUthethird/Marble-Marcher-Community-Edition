#version 430
//4*4 ray bundle
#define group_size 8
#define buffer_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D bloom; 
layout(rgba8, binding = 1) uniform image2D final_color; 
layout(rgba32f, binding = 2) uniform image2D DE_input; 
layout(rgba32f, binding = 3) uniform image2D color_HDR; 

//??
shared vec4 de_sph[1][1]; 

#include<camera.glsl>
#include<shading.glsl>

#define VIGNETTE_STRENGTH 0.2

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	vec2 img_size = vec2(imageSize(color_HDR));
	
	vec2 res_ratio = vec2(imageSize(bloom))/img_size;
	vec3 bloom_color = interp(bloom, vec2(global_pos)*res_ratio).xyz;
	
	vec3 fin_color = imageLoad(color_HDR, global_pos).xyz + bloom_color;
	
	float vignette = 1.0 - VIGNETTE_STRENGTH * length(vec2(global_pos)/img_size - 0.5);
	imageStore(final_color, global_pos,  vec4(HDRmapping(fin_color, Camera.exposure, 2.2)*vignette, 1));	 
}