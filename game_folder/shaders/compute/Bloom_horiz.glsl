#version 430
//4*4 ray bundle
#define group_size 32
#define buffer_size 64
#define block_size 64

layout(local_size_x = group_size, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D color; 
layout(rgba32f, binding = 1) uniform image2D blurred_hor1; //calculate bloom
layout(rgba32f, binding = 2) uniform image2D blurred_hor2; //calculate blur
layout(rgba32f, binding = 3) uniform image2D DE_input; 
layout(rgba32f, binding = 4) uniform image2D color_HDR; //calculate final color

shared vec4 color_buffer[buffer_size];

int safe_coord(int x, vec2 img_size)
{
	if(x >= 0 && x<img_size.x)
	{
		return x;
	}
	else return -1;
}

int shift(int x)
{
	if(x<group_size/2)
	{
		return -group_size/2;
	}
	else
	{
		return group_size/2;
	}
}

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	vec2 img_size = vec2(imageSize(color));
	
	//load a linear chunk of the image
	color_buffer[local_indx.x+group_size/2] = imageLoad(color, global_pos);
	int new_coord = safe_coord(global_pos.x + shift(local_indx.x), img_size);
	color_buffer[local_indx.x + group_size/2 + shift(local_indx.x)] = (new_coord!=-1)?imageLoad(color, global_pos):0;
	memoryBarrierShared(); 
	barrier();
	
	

	
	imageStore(color_output, global_pos,  vec4(HDRmapping(color.xyz, Camera.exposure, 2.2)*vignette);
}