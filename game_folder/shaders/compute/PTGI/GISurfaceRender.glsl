#version 430
#define group_size 8
#define block_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;

layout(rgba32f, binding = 0) uniform image2D shadows_in; 
layout(rgba32f, binding = 1) uniform image2D color_out; 
layout(rgba32f, binding = 2) uniform image2D DE_input; //final color
layout(rgba32f, binding = 3) uniform image2D DE_previous; 
layout(rgba32f, binding = 4) uniform image2D normals; 
layout(rgba32f, binding = 5) uniform image2D HDR0;  
layout(rgba32f, binding = 6) uniform image2D HDR1; 
layout(rgba32f, binding = 7) uniform image2D GI;

#include<utility/definitions.glsl>
#include<utility/uniforms.glsl>
#include<utility/camera.glsl>
#include<utility/shading.glsl>

//data block
shared vec4 indirB[group_size*group_size];
shared vec4 inposB[group_size*group_size];
shared vec4 inillB[group_size*group_size];
shared vec4 innormB[group_size*group_size];

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	ivec2 group_center = global_pos - local_indx + group_size/2;
	vec2 img_size = vec2(imageSize(DE_input));
	vec2 pimg_size = 8*ceil(img_size/(8.*GI_SCALE));
	vec2 step_scale = img_size/pimg_size;
	vec2 res_ratio = vec2(imageSize(shadows_in))/img_size;
	
	vec4 sph = imageLoad(DE_input, global_pos);
	vec4 norm = imageLoad(normals, global_pos);
	
	//random sample shift
	ivec2 shift = 0*ivec2(group_size*(hash21(float(iFrame%1000)*123.5 + dot(group_center,vec2(1., SQRT3)))-0.5))/2;
	
	//load a chunk of the GI to the shared workgroup GPU memory
	ivec2 tpos = ivec2(ceil(vec2(group_center)/step_scale)) + local_indx - group_size/2 + shift;
	int unit = local_indx.x + local_indx.y*group_size;
	indirB[unit] = subImage(GI, tpos, ivec2(0,0), ivec2(pimg_size));
	inposB[unit] = subImage(GI, tpos, ivec2(1,0), ivec2(pimg_size));
	inillB[unit] = subImage(GI, tpos, ivec2(2,0), ivec2(pimg_size));
	innormB[unit] = subImage(GI, tpos, ivec2(3,0), ivec2(pimg_size));
	
	indirB[unit].xyz=indirB[unit].xyz/(length(indirB[unit].xyz)+0.00001);
	inillB[unit].xyz/=inillB[unit].w;
	
	//sync workgroup threads
	memoryBarrierShared(); 
	barrier();

	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	vec4 var = vec4(0);
	
	float td = dot(dir.xyz, sph.xyz - pos.xyz);//traveled distance
	pos = sph;
	dir.w += td; 
	
	vec3 color = vec3(0.);
	
	//path trace sample radius
	float pixsize = 3.*td*fovray*step_scale.x;
	
	if(pos.w < -max(2*fovray*td, MIN_DIST))
	{
		vec4 albedo; vec2 pbr; vec3 emission;
		scene_material(pos.xyz, albedo, pbr, emission);
		color = albedo.xyz;
	}
	else if(pos.w < max(2*fovray*td, MIN_DIST))
	{
		float k = 0.0001;
		vec3 cpos = pos.xyz - pos.w*norm.xyz;
		vec4 albedo; vec2 pbr; vec3 emission;
		scene_material(cpos, albedo, pbr, emission);
		pixsize = min(pixsize/(abs(dot(norm.xyz, dir.xyz)) + 0.1), 4.*td*fovray*step_scale.x);
		vec3 GILo = vec3(0.);
		//computing the irradiance probe using the stored PT samples, pretty slow 
		for(int i = 0; i<group_size*group_size; i++)
		{
			//get input ray and surface data
			vec3 P =  inposB[i].xyz;
			vec3 N =  innormB[i].xyz;
			
			//bilateral weight over the ray sample
			vec3 dP = (cpos - P)/pixsize;
			float dN = length(norm.xyz - N)*5.;
			float weight = exp(-dot(dP,dP)-dN*dN);
			
			if(weight > 0.05*k)
			{
				vec3 Ri = indirB[i].xyz;
				vec3 Li = inillB[i].xyz;
				GILo += BRDF(-dir.xyz, Ri, norm.xyz, albedo.xyz, pbr)*Li*weight;
				k += weight;
			}
		}
		GILo/=k;
		vec4 AO = ambient_occlusion(pos, norm, dir);
		GILo *= 0.1 + 0.9*AO.w;
		
		if(albedo.w > 0.5)
		{
			color = marble_render(vec4(cpos, norm.w), dir, norm, fovray, GILo);
		}
		else
		{
			color += GILo;
			//shadowmapping
			vec3 shadows = sky_color(normalize(LIGHT_DIRECTION))*bilinear_surface(shadows_in, td, 3*td*fovray/res_ratio.x, vec2(global_pos)*res_ratio).xyz;
			
			float NdotL = max(dot(norm.xyz, normalize(LIGHT_DIRECTION)), 0.0);       
			color += BRDF(-dir.xyz, normalize(LIGHT_DIRECTION), norm.xyz, albedo.xyz, pbr)*shadows*NdotL;
			
			color += emission;
		}
		
	}
	else
	{
		color = sky_color(dir.xyz);
	}
	
	if(!isnan(color.x) && !isnan(color.y) && !isnan(color.z))
	{
		imageStore(color_out, global_pos, vec4(color.xyz, 1.));	 
	}
	
	//swap old GI data for new
	if(global_pos.y < pimg_size.y && global_pos.x < pimg_size.x)
	{
		vec4 pdir = subImage(GI, global_pos, ivec2(0,0), ivec2(pimg_size));
		vec4 ppos = subImage(GI, global_pos, ivec2(1,0), ivec2(pimg_size));
		vec4 pill = subImage(GI, global_pos, ivec2(2,0), ivec2(pimg_size));
		vec4 pnorm = subImage(GI, global_pos, ivec2(3,0), ivec2(pimg_size));
		storeSub(GI, pdir, global_pos, ivec2(0,1), ivec2(pimg_size));
	 	storeSub(GI, ppos, global_pos, ivec2(1,1), ivec2(pimg_size));
		storeSub(GI, pill, global_pos, ivec2(2,1), ivec2(pimg_size));		
		storeSub(GI, pnorm, global_pos, ivec2(3,1), ivec2(img_size));		
	}
}