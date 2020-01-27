#version 430
#define group_size 8
#define block_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D global_illum; 
layout(rgba32f, binding = 1) uniform image2D illuminationDirect; 
layout(rgba32f, binding = 2) uniform image2D illuminationGI; 
layout(rgba32f, binding = 3) uniform image2D outnormals; 
layout(rgba32f, binding = 4) uniform image2D DE_input; 
layout(rgba32f, binding = 5) uniform image2D color_HDR; //calculate final color

//make all the local distance estimator spheres shared
shared vec4 de_sph[group_size][group_size]; 

#include<utility/camera.glsl>
#include<utility/shading.glsl>

///Low-resolution illumination step

void main() {

	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	
	vec2 img_size = vec2(imageSize(illuminationDirect));
	vec2 pimg_size = vec2(imageSize(DE_input));
	float GIscale = (vec2(imageSize(global_illum)) / img_size).x;
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
	
	vec4 illumDIR = vec4(0);
	vec4 illumGI = vec4(0);
	vec3 normal = vec3(1.,0.,0.);
	
	if(pos.w < max(5*fovray*td, MIN_DIST) && SHADOWS_ENABLED)
	{
		illumGI = bilinear_surface(global_illum, td, 16.*td*fovray/GIscale.x, vec2(global_pos)*GIscale);
		//marching towards a point at a distance = to the pixel cone radius from the object
		float pix_cone_rad = 3.*fovray*td/step_scale.x;
		pos.xyz += (DE(pos.xyz) - pix_cone_rad)*dir.xyz;
		pos.xyz += (DE(pos.xyz) - pix_cone_rad)*dir.xyz;
		pos.xyz += (DE(pos.xyz) - pix_cone_rad)*dir.xyz;
		normal = normalize(calcNormal(pos.xyz, pix_cone_rad*0.5).xyz);
	
		illumDIR.xyz = sky_color(normalize(LIGHT_DIRECTION))*shadow_march(pos, normalize(vec4(LIGHT_DIRECTION,0)), MAX_DIST, LIGHT_ANGLE);
	}
	illumDIR.w = td;
	illumGI.w = td;
	
	imageStore(illuminationDirect, global_pos, illumDIR);
	imageStore(illuminationGI, global_pos, illumGI);	
	imageStore(outnormals, global_pos, vec4(normal,1.));	
}