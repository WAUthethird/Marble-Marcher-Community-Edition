#version 430
//4*4 ray bundle
#define group_size 8
#define buffer_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D before; 
layout(rgba32f, binding = 1) uniform image2D after; 
layout(rgba32f, binding = 2) uniform image2D DE_input; 

//bilateral blur... 
//4 pixel radius
#define blur_R 1

vec3 getpos(vec2 p, vec2 rr)
{
	return imageLoad(DE_input, ivec2(p*rr)).xyz;
}

#include<utility/camera.glsl>

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	
	vec2 img_size = vec2(imageSize(before));
	
	vec2 res_ratio = vec2(imageSize(DE_input))/img_size;
	
	vec4 sum = vec4(0);
	float norm = 0.;
	
	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	vec4 var = vec4(0);
	
	vec3 cpos = getpos(global_pos, res_ratio);
	float td = dot(dir.xyz, cpos - pos.xyz);//traveled distance
	float DX = fovray*td;
	
	for(int i = -blur_R; i <= blur_R; i++)
	{
		for(int j = -blur_R; j <= blur_R; j++)
		{
			vec3 dpos = (getpos(global_pos + vec2(i,j),res_ratio) - cpos)/td;
			float weight = exp(- 1200.*dot(dpos,dpos));
			sum += weight*imageLoad(before, ivec2(global_pos) + ivec2(i,j));
			norm += weight;
		}
	}
	sum /= norm; 
	
	imageStore(after, global_pos,  sum);
}