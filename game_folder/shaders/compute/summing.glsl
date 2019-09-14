#version 430
//4*4 ray bundle
#define group_size 16

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D high_res; 
layout(rgba32f, binding = 1) uniform image2D low_res; 

#define scaling 3

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	
	vec4 sum = vec4(0);

	#pragma unroll
	for(int i = 0; i < scaling; i++)
	{
		#pragma unroll
		for(int j = 0; j < scaling; j++)
		{
			sum += imageLoad(high_res, ivec2(global_pos*res_ratio) + ivec2(i,j));
		}
	}
	
	imageStore(low_res, global_pos,  sum);
}