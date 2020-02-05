#version 430
#define group_size 8
#define block_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D illuminationDirect; 
layout(rgba32f, binding = 1) uniform image2D color_output; 
layout(rgba32f, binding = 2) uniform image2D DE_input; 
layout(rgba32f, binding = 3) uniform image2D DE_previous; //calculate final color
layout(rgba32f, binding = 4) uniform image2D normals; //final color
layout(rgba32f, binding = 5) uniform image2D color_HDR0;  
layout(rgba32f, binding = 6) uniform image2D color_HDR1; 
//layout(rgba32f, binding = 6) uniform image2D GI; 

#include<utility/definitions.glsl>
#include<utility/uniforms.glsl>
#include<utility/camera.glsl>
#include<utility/shading.glsl>

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	vec2 img_size = vec2(imageSize(color_HDR0));
	float res_ratio = imageSize(illuminationDirect).x/img_size.x;
	vec4 sph = imageLoad(DE_input, global_pos);
	
	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,0);
	vec3 cpos = pos.xyz;
	vec4 dir = vec4(rr.dir,0);
	vec4 var = vec4(0);
	
	float td = dot(dir.xyz, sph.xyz - pos.xyz);//traveled distance
	pos = sph;
	dir.w += td; 
	
	vec3 color = shading(pos, dir, fovray, illuminationDirect, vec3(vec2(global_pos)*res_ratio, 2.*td*fovray/res_ratio));

	if(!isnan(color.x) && !isnan(color.y) && !isnan(color.z))
	{
		imageStore(color_HDR1, global_pos, vec4(color.xyz, 1.));	 
		imageStore(color_output, global_pos, vec4(color.xyz, td));	 
	}
}