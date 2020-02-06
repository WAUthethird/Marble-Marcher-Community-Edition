#include<utility/compute_shader_header.glsl>

layout(rgba32f, binding = 0) uniform image2D DE_input; 
layout(rgba32f, binding = 1) uniform image2D DE2_input;
layout(rgba32f, binding = 2) uniform image2D var_input; 
layout(rgba32f, binding = 3) uniform image2D DE_output; //calculate final DE spheres
layout(rgba32f, binding = 4) uniform image2D DE_previus; //previous 
layout(rgba32f, binding = 5) uniform image2D normals; 


#include<utility/definitions.glsl>
#include<utility/uniforms.glsl>
#include<utility/camera.glsl>
#include<utility/shading.glsl>

///The second step of multi resolution ray marching

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	vec2 img_size = vec2(imageSize(DE_output));
	vec2 pimg_size = vec2(imageSize(DE_input));
	vec2 MRRM_step_scale = img_size/pimg_size;
	//if within the texture
	
	ivec2 prev_pos = min(ivec2((vec2(global_pos)/MRRM_step_scale) + 0.5),ivec2(pimg_size)-1);
	//initialize the ray
	vec4 sph = imageLoad(DE_input, prev_pos);
	vec4 sph_norm = imageLoad(DE2_input, prev_pos);
	
	#if(RBM1)
		de_sph[local_indx.x][local_indx.y] = sph;
		memoryBarrierShared(); 
	#endif
	
	vec2 uv = vec2(global_pos)/img_size;
	ray rr = get_ray(uv, 1.);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	vec4 var = imageLoad(var_input, prev_pos);
		
	float td = dot(dir.xyz, sph.xyz - pos.xyz);//traveled distance
	
	//first order, MRRM
	pos.xyz += dir.xyz*td;//move local ray beginning inside the DE sphere	
	dir.w += td; 

	pos.w = 0.5*sphere_intersection(dir.xyz, pos.xyz, sph);
	//pos.w = max(pos.w, sphere_intersection(dir.xyz, pos.xyz, sph_norm));
	
	ray_march_continue(pos, dir, var, fovray);
	
	//save the DE spheres
	imageStore(DE_previus, global_pos, imageLoad(DE_output, global_pos));
	
	vec4 normal = calcNormal(pos.xyz, max(MIN_DIST, 0.5*dir.w*fovray));
	//pos.xyz += normalize(normal.xyz)*DE(pos.xyz);
	imageStore(DE_output, global_pos, pos);	 
	
	imageStore(normals, global_pos, vec4(normalize(normal.xyz), dir.w));	 	
}