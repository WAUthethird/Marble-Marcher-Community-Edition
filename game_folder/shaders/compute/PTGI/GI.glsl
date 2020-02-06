#include<utility/compute_shader_header.glsl>

layout(rgba32f, binding = 0) uniform image2D DE_input; //final color
layout(rgba32f, binding = 1) uniform image2D DE_previous; 
layout(rgba32f, binding = 2) uniform image2D normals; 
layout(rgba32f, binding = 3) uniform image2D HDR0;  
layout(rgba32f, binding = 4) uniform image2D HDR1; 
layout(rgba32f, binding = 5) uniform image2D GI;


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
	
	vec2 img_size = gl_NumWorkGroups.xy*group_size;
	vec2 pimg_size = vec2(imageSize(DE_input));
	vec2 step_scale = img_size/pimg_size;
	
	vec2 uv = (vec2(global_pos)+0.5*(hash22(vec2(global_pos))-0.5))/img_size;
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
	
	norm = calcNormal(pos.xyz, td*fovray);
	norm.xyz = normalize(norm.xyz);
	
	vec3 illum = vec3(0);
	
	//light field reprojection
	vec2 lastCoord = reproject(pos.xyz, uv)*step_scale;
	vec4 pdir = subInterp(GI, lastCoord, ivec2(0,1), ivec2(img_size));
	vec4 ppos = subInterp(GI, lastCoord, ivec2(1,1), ivec2(img_size));
	vec4 pill = subInterp(GI, lastCoord, ivec2(2,1), ivec2(img_size));
	vec4 pnorm = subInterp(GI, lastCoord, ivec2(3,1), ivec2(img_size));
	
	vec4 lastPos = interp(DE_previous, round(lastCoord/step_scale));
	vec2 lastUV = project(lastPos.xyz, uv);
	float delta = pimg_size.x*length(lastUV/pimg_size - uv);
	float dp = length(ppos.xyz - pos.xyz)*step_scale.x/(td*fovray);
	
	float removeK = exp(-0.001*pow(delta, 2.));//remove prev data based on relative pixel distance
	removeK *= exp(-0.005*pow(dp, 2.));
	float trshd = 0.5*tanh(0.2*(pill.w - 8)) + 0.5; //dont remove if theres not enough samples - minimizes visible noise
	removeK = 1. - trshd + trshd*removeK;
	pill *= removeK;
	pdir *= removeK;
	
	if(pos.w < -max(2*fovray*td, MIN_DIST))
	{
		pill = vec4(0.,0.,0.,1.);
	}
	else if(pos.w < max(2*fovray*td, MIN_DIST))
	{
		float seed = dot(global_pos,vec2(1., SQRT3)) + float(iFrame%1000)*123.5;
		
		for(int i = 0; i < PT_SAMPLES; i++)
		{
			vec4 ddir = dir; 
			illum = path_march(pos, ddir, vec4(120,0,0,0), step_scale.x*fovray, seed, 1, 1);
			#ifdef LIGHT_FIELD_DENOISE
				pill.xyz = pill.xyz + illum;
				pill.w += 1.;
				pdir.xyz = pdir.xyz + ddir.xyz*length(illum.xyz);
				pdir.w += 1.;
			#else
				pill.xyz = illum;
				pill.w = 1.;
				pdir.xyz =  ddir.xyz;
				pdir.w = 1.;
			#endif
		}
	}
	
	if(iFrame < 1)
	{
		pill.xyz = illum;
		pill.w = 1.;
		pdir.xyz = dir.xyz;
	}
	
	if(isnan(pdir.x) || isnan(pdir.y) || isnan(pdir.z)) pdir = vec4(1.,0.,0.,1.);
	if(isnan(pill.x) || isnan(pill.y) || isnan(pill.z)) pill = vec4(0);
	
	ppos.xyz = pos.xyz;
	pnorm.xyz = norm.xyz;
	
	if(!isnan(illum.x) && !isnan(illum.y) && !isnan(illum.z))
	{
		//save normals, position, light direction and illumination
		storeSub(GI, pdir, global_pos, ivec2(0,0), ivec2(img_size));
		storeSub(GI, ppos, global_pos, ivec2(1,0), ivec2(img_size));
		storeSub(GI, pill, global_pos, ivec2(2,0), ivec2(img_size));	
		storeSub(GI, pnorm, global_pos, ivec2(3,0), ivec2(img_size));
	}
			
}