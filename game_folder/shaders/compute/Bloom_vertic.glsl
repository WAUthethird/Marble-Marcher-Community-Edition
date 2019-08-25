#version 430
//4*4 ray bundle
#define group_size 128
#define buffer_size 256

layout(local_size_x = 1, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D blurred_hor1; 
layout(rgba32f, binding = 1) uniform image2D blurred_hor2; 
layout(rgba8, binding = 2) uniform image2D final_color; 
layout(rgba32f, binding = 3) uniform image2D DE_input; 
layout(rgba32f, binding = 4) uniform image2D color_HDR; 

shared vec3 color_buffer_hoz1[buffer_size];
shared vec3 color_buffer_hoz2[buffer_size];

//??
shared vec4 de_sph[1][1]; 

#include<camera.glsl>
#include<shading.glsl>

#define VIGNETTE_STRENGTH 0.2

int safe_coord(int y, vec2 img_size)
{
	if(y >= 0 && y<img_size.y)
	{
		return y;
	}
	else return -1;
}

int shift(int y)
{
	if(y<group_size/2)
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
	vec2 img_size = vec2(imageSize(color_HDR));
	
	//load a linear chunks of the image
	color_buffer_hoz1[local_indx.y + group_size/2] = imageLoad(blurred_hor1, global_pos).xyz;
	int new_coord = safe_coord(global_pos.y + shift(local_indx.y), img_size);
	color_buffer_hoz1[local_indx.y + group_size/2 + shift(local_indx.y)] =
	(new_coord!=-1)?imageLoad(blurred_hor1, ivec2(global_pos.x,new_coord)).xyz:0;
	
	color_buffer_hoz2[local_indx.y + group_size/2] = imageLoad(blurred_hor2, global_pos).xyz;
	new_coord = safe_coord(global_pos.y + shift(local_indx.y), img_size);
	color_buffer_hoz2[local_indx.y + group_size/2 + shift(local_indx.y)] =
	(new_coord!=-1)?imageLoad(blurred_hor2, ivec2(global_pos.x,new_coord)).xyz:0;
	
	memoryBarrierShared(); 
	barrier();
	
	float width = 2*img_size.x/640;
	float k1 = 0.1;
	float k2 = 1;
	float a1 = 0.4;
	float a2 = 1;
	
	vec3 conv1 = vec3(0), conv2 = vec3(0);
	for(int i = 0; i < buffer_size; i++)
	{
		float coord = float(local_indx.y+group_size/2-i)/width;
		conv1 += exp(-k1*pow(coord,2))*color_buffer_hoz1[i];
		conv2 += exp(-k2*pow(coord,2))*color_buffer_hoz2[i];
	}
	
	vec3 bloom = (a1*conv1+a2*conv2)/(a1+a2);
	vec3 fin_color = imageLoad(color_HDR, global_pos).xyz + 0.2*bloom/(width*width);
	
	float vignette = 1.0 - VIGNETTE_STRENGTH * length(vec2(global_pos)/img_size - 0.5);
	imageStore(final_color, global_pos,  vec4(HDRmapping(fin_color, Camera.exposure, 2.2)*vignette, 1));	 
}