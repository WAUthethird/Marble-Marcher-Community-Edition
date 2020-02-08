#include<utility/compute_shader_header.glsl>

layout(rgba32f, binding = 0) uniform image2D PTout; //final color
layout(rgba32f, binding = 1) uniform image2D DE_input; 
layout(rgba32f, binding = 2) uniform image2D DE_previous; 
layout(rgba32f, binding = 3) uniform image2D normals; 
layout(rgba32f, binding = 4) uniform image2D HDR0;  
layout(rgba32f, binding = 5) uniform image2D HDR1; 
layout(rgba32f, binding = 6) uniform image2D GI;


#include<utility/definitions.glsl>
#include<utility/uniforms.glsl>
#include<utility/camera.glsl>
#include<utility/shading.glsl>
#include<utility/path_tracing.glsl>

/// 1/6 resolution path tracing 

#define PT_SAMPLES 1

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	
	vec2 img_size = vec2(imageSize(PTout));
	vec2 pimg_size = vec2(imageSize(DE_input));
	vec2 step_scale = img_size/pimg_size;
	
	vec2 uv = vec2(global_pos)/img_size;
	ray rr = get_ray(uv);
	ivec2 prev_pos = min(ivec2(round(uv*pimg_size)),ivec2(pimg_size)-1);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	
	vec4 sph = imageLoad(DE_input, prev_pos);
	vec4 norm = imageLoad(normals, prev_pos);
	float td = dot(dir.xyz, sph.xyz - pos.xyz);//traveled distance
	
	dir.xyz = normalize(sph.xyz - pos.xyz);
	pos = sph;
	dir.w += td; 
	
	vec3 illum = vec3(0.);
	
	if(pos.w < -max(2*fovray*td, MIN_DIST))
	{
		illum = vec3(0.,0.,0.);
	}
	else if(pos.w < max(2*fovray*td, MIN_DIST))
	{
		float seed = dot(global_pos, vec2(SQRT2, SQRT3)) + float(iFrame%1000)*123.5;
		
		for(int i = 0; i < PT_SAMPLES; i++)
		{
			vec4 ddir = dir; 
			illum += path_march(pos, ddir, vec4(120,0,0,0), 0.25*fovray, seed+i, 0, 0);
		}
		illum /= float(PT_SAMPLES);
	}
	else
	{
		illum = sky_color(dir.xyz);
	}
	
	
	if(!isnan(illum.x) && !isnan(illum.y) && !isnan(illum.z))
	{
		imageStore(PTout, global_pos, vec4(illum, 1.));
	}
			
}