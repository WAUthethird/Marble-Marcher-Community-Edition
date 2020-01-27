#version 430
//4*4 ray bundle
#define group_size 8
#define buffer_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D color_input; 
layout(rgba32f, binding = 1) uniform image2D color_output; 
layout(rgba32f, binding = 2) uniform image2D DE_input; 
//layout(rgba32f, binding = 3) uniform image2D color_HDR0; //prev color
layout(rgba32f, binding = 4) uniform image2D prevDE; //calculate final color
//layout(rgba32f, binding = 5) uniform image2D color_HDR1; //final color
layout(rgba32f, binding = 6) uniform image2D GIdata0;  
layout(rgba32f, binding = 7) uniform image2D GIdata1; 

// Temporal Denoiser With Reprojection

//make all the local distance estimator spheres shared
shared vec4 de_sph[group_size][group_size]; 

#include<utility/camera.glsl>
#include<utility/shading.glsl>

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	
	vec2 img_size = vec2(imageSize(color_input));
	vec2 res_ratio = vec2(imageSize(DE_input))/img_size;
	res_ratio = vec2(res_ratio.x, round(res_ratio.y)); 
	
	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	vec4 var = vec4(0);
	
	vec4 sph = imageLoad(DE_input, ivec2((global_pos)*res_ratio));
	float td = length(sph.xyz - pos.xyz);//traveled distance
	
    //getting the previous frame pixel and sampling it bicubically 
	vec2 lastCoord = reproject(pos.xyz + td*dir.xyz, (vec2(global_pos))/img_size);
	
    vec4 lastColor = interp_bicubic(GIdata0, clamp(lastCoord/res_ratio,vec2(2.),img_size-2.));
	vec4 lastPos = interp(prevDE, lastCoord);
	
    vec3 in0 = imageLoad(color_input, global_pos).xyz;
	
	//rejecting some of the previous data
	float delta = min_distance(prevDE, sph.xyz, lastCoord, 0)/(max(td+4.*Camera.size,MIN_DIST)*fovray);
	lastColor *= exp(-0.001*pow(delta, 2.))*(0.96 + 0.039*tanh(4. - length( global_pos*res_ratio - lastCoord )));
	
	if(iFrame < 1 || isnan(lastColor.x)) lastColor = vec4(0.);
	
	//adding new pixel
	lastColor.xyz += in0;
	lastColor.w += 1.;
	
	imageStore(color_output, global_pos, vec4(lastColor.xyz/lastColor.w, td));
	imageStore(GIdata1, global_pos, vec4(lastColor));
}