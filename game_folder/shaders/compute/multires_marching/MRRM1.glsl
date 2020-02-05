#include<utility/compute_shader_header.glsl>

layout(rgba32f, binding = 0) uniform image2D DE_output; //calculate final DE spheres
layout(rgba32f, binding = 1) uniform image2D DE2_output;
layout(rgba32f, binding = 2) uniform image2D var_output; 
layout(rgba32f, binding = 3) uniform image2D DE_input; 
layout(rgba32f, binding = 4) uniform image2D color_HDR; //calculate final color

#include<utility/definitions.glsl>
#include<utility/uniforms.glsl>
#include<utility/camera.glsl>
#include<utility/ray_marching.glsl>

///The first step of multi resolution ray marching

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.x,gl_GlobalInvocationID.y);
	int local_indx = int(gl_LocalInvocationIndex);
	vec2 img_size = vec2(imageSize(DE_output));
	//if within the texture
	
	//initialize the ray
	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	vec4 var = vec4(0);
	float res_ratio = float(imageSize(color_HDR).x/imageSize(DE_output).x);
	fovray *= res_ratio;
	dir.w = 4*Camera.size;
	ray_march_limited(pos, dir, var, 2.4*fovray);
	vec4 pos1 = pos;
	
	normarch(pos1);
	
	//save the DE spheres
	imageStore(DE_output, global_pos, pos);	 
	imageStore(DE2_output, global_pos, pos1);	 
	imageStore(var_output, global_pos, var);			
}