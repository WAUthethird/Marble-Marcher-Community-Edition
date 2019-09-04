#version 430
//4*4 ray bundle
#define group_size 8
#define block_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba8, binding = 0) uniform image2D final_color;

#include<camera.glsl>
#include<original_functions.glsl>

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	vec2 img_size = vec2(imageSize(final_color));
	
	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,1);
	vec4 dir = vec4(rr.dir,0);
	vec4 var = vec4(0);
	
	vec3 col = vec3(0.0);
	
	float vignette = 1.0 - VIGNETTE_STRENGTH * length(vec2(global_pos)/img_size - 0.5);

	vec4 col_r = scene(pos, dir, vignette);

	//Check if this is the glass marble
	if (col_r.w > 0.5) {
		//Calculate refraction
		vec3 r = -dir.xyz;
		vec3 n = normalize(iMarblePos - pos.xyz);
		vec3 q = refraction(r, n, 1.0 / 1.5);
		vec3 p2 = pos.xyz + (dot(q, n) * 2.0 * iMarbleRad) * q;
		n = normalize(p2 - iMarblePos);
		q = (dot(q, r) * 2.0) * q - r;
		vec4 r_temp = vec4(q, 0.0);
		vec4 p_temp = vec4(p2 + n * (MIN_DIST * 10), 1.0);
		vec3 refr = scene(p_temp, r_temp, 0.8).xyz;

		//Calculate refraction
		n = normalize(pos.xyz - iMarblePos);
		q = r - n*(2*dot(r,n));
		r_temp = vec4(q, 0.0);
		p_temp = vec4(pos.xyz + n * (MIN_DIST * 10), 1.0);
		vec3 refl = scene(p_temp, r_temp, 0.8).xyz;

		//Combine for final marble color
		col += refr * 0.6f + refl * 0.4f + col_r.xyz;
	} else {
		col += col_r.xyz;
	}
	
	
	imageStore(final_color, global_pos,  vec4(2*col*Camera.exposure, 1));	 	 
}