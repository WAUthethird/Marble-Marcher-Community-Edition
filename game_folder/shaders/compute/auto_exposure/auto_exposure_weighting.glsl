#version 430

/// DO NOT CHANGE
#define group_size 16

layout(local_size_x = group_size, local_size_y = group_size) in;

layout(rgba8, binding = 0) uniform image2D final_color; 
layout(rgba32f, binding = 1) uniform image2D DE_input; 
layout(rgba32f, binding = 2) uniform image2D output_w; 

#include<utility/definitions.glsl>
#include<utility/uniforms.glsl>
#include<utility/camera.glsl>

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	vec2 img_size = vec2(imageSize(final_color));
	
	vec4 sph = imageLoad(DE_input, global_pos);

	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	
	float td = dot(dir.xyz, sph.xyz - pos.xyz);//depth
	
	vec4 color = imageLoad(final_color, global_pos);
	float weight = pow(max(1.0 - length(vec2(global_pos)/img_size - 0.5), 0.f),6);
	weight *= sqrt(abs(1.f/(td + Camera.size + 1e-5)));
	imageStore(output_w, global_pos,  vec4(max(color.x,max(color.y,color.z))*weight, weight, 0, 1));
}