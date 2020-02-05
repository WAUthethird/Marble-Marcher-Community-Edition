#version 430
//4*4 ray bundle
#define group_size 8
#define block_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D illumination; //shadows
layout(rgba32f, binding = 1) uniform image2D color_output; 
layout(rgba32f, binding = 2) uniform image2D DE_input; 
layout(rgba32f, binding = 3) uniform image2D color_HDR; //calculate final color


#include<utility/definitions.glsl>
#include<utility/uniforms.glsl>
#include<utility/camera.glsl>
#include<utility/shading.glsl>
#include<original/original_functions.glsl>

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	vec2 img_size = vec2(imageSize(color_HDR));
	float res_ratio = imageSize(illumination).x/img_size.x;
	vec4 sph = imageLoad(DE_input, global_pos);
	
	
	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	vec4 var = vec4(0);
	
	float td = dot(dir.xyz, sph.xyz - pos.xyz);//traveled distance
	
	vec4 illum = bilinear_surface(illumination, td, 3*td*fovray/res_ratio, vec2(global_pos)*res_ratio);
	//vec4 illum = interp(illumination, vec2(global_pos)*res_ratio);
	pos = sph;
	dir.w += td; 
	
	vec3 color = shading_orig(pos, dir, fovray, illum.x);
	
	vec3 prev_color = imageLoad(color_HDR, global_pos).xyz;
	if(!isnan(color.x) && !isnan(color.y) && !isnan(color.z))
	{
		//color = prev_color*Camera.mblur + (1-Camera.mblur)*color; //blur
		imageStore(color_HDR, global_pos, vec4(color.xyz, td));	 
		imageStore(color_output, global_pos, vec4(color.xyz, 1.));	 
	}
}