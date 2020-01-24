#version 430
#define group_size 8
#define block_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D global_illum; 
layout(rgba32f, binding = 1) uniform image2D normals; 
layout(rgba32f, binding = 2) uniform image2D DE_input; 
layout(rgba32f, binding = 3) uniform image2D color_HDR; //calculate final color

//make all the local distance estimator spheres shared
shared vec4 de_sph[group_size][group_size]; 

#include<utility/camera.glsl>
#include<utility/shading.glsl>
#include<utility/path_tracing.glsl>

///Half-resolution global illumination step

void main() {

	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	
	vec2 img_size = vec2(imageSize(global_illum));
	vec2 pimg_size = vec2(imageSize(DE_input));
	vec2 step_scale = img_size/pimg_size;
	
	ivec2 prev_pos = min(ivec2((vec2(global_pos)/step_scale) + 0.5),ivec2(pimg_size)-1);
	
	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	vec4 var = vec4(0);
	
	vec4 sph = imageLoad(DE_input, prev_pos);
	float td = dot(dir.xyz, sph.xyz - pos.xyz);//traveled distance
	
	pos = sph;
	dir.w += td; 
	
	vec4 illum = vec4(0);
	vec4 norm = vec4(1,0,0,0);
	if(pos.w < max(2*fovray*td, MIN_DIST) && SHADOWS_ENABLED)
	{
		//marching towards a point at a distance = to the pixel cone radius from the object
		float pix_cone_rad = 2.*fovray*td/step_scale.x;
		pos.xyz += (DE(pos.xyz) - pix_cone_rad)*dir.xyz;
		pos.xyz += (DE(pos.xyz) - pix_cone_rad)*dir.xyz;
		pos.xyz += (DE(pos.xyz) - pix_cone_rad)*dir.xyz;
		norm = calcNormal(pos.xyz, pix_cone_rad);
		float seed = dot(global_pos,vec2(1., SQRT3)) + float(iFrame%1000)*123.5;
		illum.xyz = path_march(pos, dir, vec4(80,0,0,0), fovray/step_scale.x, seed);
	}
	illum.w = td;
	imageStore(global_illum, global_pos, illum);	 
	imageStore(normals, global_pos, norm);	 
}