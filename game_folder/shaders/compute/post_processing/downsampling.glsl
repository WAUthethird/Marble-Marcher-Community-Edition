#version 430
//4*4 ray bundle
#define group_size 8
#define buffer_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D high_res; 
layout(rgba32f, binding = 1) uniform image2D low_res; 

//downsamples the texture from the previous step

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	
	vec2 img_size = vec2(imageSize(low_res));
	
	vec2 res_ratio = vec2(imageSize(high_res))/img_size;
	
	vec4 sum = vec4(0);
	if(res_ratio.x > 1 || res_ratio.y > 1)
	{
		for(int i = 0; i < res_ratio.x; i++)
		{
			for(int j = 0; j < res_ratio.y; j++)
			{
				sum += imageLoad(high_res, ivec2(global_pos*res_ratio) + ivec2(i,j));
			}
		}
		sum /= res_ratio.x*res_ratio.y; 
	}
	else
	{
		sum = imageLoad(high_res, global_pos);
	}
	
	imageStore(low_res, global_pos,  sum);
}