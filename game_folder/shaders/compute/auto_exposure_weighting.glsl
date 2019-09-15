#version 430

/// DO NOT CHANGE
#define group_size 16

layout(local_size_x = group_size, local_size_y = group_size) in;

layout(rgba8, binding = 0) uniform image2D final_color; 
layout(rgba32f, binding = 1) uniform image2D output_w; 

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	vec2 img_size = vec2(imageSize(final_color));
	
	vec4 color = imageLoad(final_color, global_pos);
	float weight = max(1.0 - length(vec2(global_pos)/img_size - 0.5), 0.f);
	imageStore(output_w, global_pos,  vec4(length(color.xyz)*weight, weight, 0, 1));
}