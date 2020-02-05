#version 430
//4*4 ray bundle
#define group_size 8
#define buffer_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba8, binding = 0) uniform image2D final_color; 
layout(rgba32f, binding = 1) uniform image2D DE_input; 
layout(rgba32f, binding = 2) uniform image2D DE_previous; //calculate final color
layout(rgba32f, binding = 3) uniform image2D normals; //final color
layout(rgba32f, binding = 4) uniform image2D color_HDR0;  
layout(rgba32f, binding = 5) uniform image2D color_HDR1; 
//layout(rgba32f, binding = 7) uniform image2D GI; 

#include<utility/definitions.glsl>
#include<utility/uniforms.glsl>
#include<utility/camera.glsl>
#include<utility/shading.glsl>

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	
	vec2 img_size = vec2(imageSize(DE_input));
	vec4 sph = imageLoad(DE_input, global_pos);
	
	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	vec4 var = vec4(0);
	
	float td = dot(dir.xyz, sph.xyz - pos.xyz);//traveled distance
	pos = sph;
	dir.w += td;
	
	vec4 color;
	
	if(pos.w < max(2*fovray*dir.w, MIN_DIST))
	{
		vec4 norm = calcNormal(pos.xyz, 2.*fovray*dir.w); 
		norm.xyz = normalize(norm.xyz);
		//optimize color sampling 
		vec3 cpos = pos.xyz - norm.w*norm.xyz;
		vec2 pbr; vec3 emission;
		scene_material(cpos, color, pbr, emission);
		color *= max(dot(normalize(LIGHT_DIRECTION),norm.xyz),0.05);
		color.xyz += emission;
	}
	else
	{
		color.xyz = sky_color(dir.xyz);
	}
	
	
	imageStore(final_color, global_pos, vec4(HDRmapping(color.xyz, Camera.exposure), 1));	 
}