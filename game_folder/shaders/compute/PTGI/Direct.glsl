#include<utility/compute_shader_header.glsl>

layout(rgba32f, binding = 0) uniform image2D shadow_out;
layout(rgba32f, binding = 1) uniform image2D DE_input; //final color
layout(rgba32f, binding = 2) uniform image2D DE_previous; 
layout(rgba32f, binding = 3) uniform image2D normals; 
layout(rgba32f, binding = 4) uniform image2D HDR0;  
layout(rgba32f, binding = 5) uniform image2D HDR1; 
layout(rgba32f, binding = 6) uniform image2D GI;

#include<utility/definitions.glsl>
#include<utility/uniforms.glsl>
#include<utility/camera.glsl>
#include<utility/shading.glsl>

/// 1/3 shadows

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	
	vec2 img_size = vec2(imageSize(shadow_out));
	vec2 pimg_size = vec2(imageSize(DE_input));
	vec2 step_scale = img_size/pimg_size;
	
	vec2 uv = vec2(global_pos)/img_size;
	ray rr = get_ray(uv, 0.);
	ivec2 prev_pos = ivec2((vec2(global_pos)/step_scale) + 0.5);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	
	vec4 sph = imageLoad(DE_input, prev_pos);
	vec4 norm = imageLoad(normals, prev_pos);
	float td = dot(dir.xyz, sph.xyz - pos.xyz);//traveled distance
	
	dir.xyz = normalize(sph.xyz - pos.xyz);
	pos = sph;
	dir.w += td; 
	
	vec3 illum = vec3(0);
	if(pos.w < max(2*fovray*td, MIN_DIST))
	{
		float pix_cone_rad = 2.*fovray*td/step_scale.x;
		pos.xyz += (DE(pos.xyz) - pix_cone_rad)*dir.xyz;
		pos.xyz += (DE(pos.xyz) - pix_cone_rad)*dir.xyz;
		pos.xyz += (DE(pos.xyz) - pix_cone_rad)*dir.xyz;
		illum = vec3(1.)*shadow_march(pos, vec4(normalize(LIGHT_DIRECTION),0), MAX_DIST, LIGHT_ANGLE);
	}
	
	//save illumination
	imageStore(shadow_out, global_pos, vec4(illum,td));
}