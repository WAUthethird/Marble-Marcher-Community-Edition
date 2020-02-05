#version 430
//4*4 ray bundle
#define group_size 128
#define buffer_size 256

layout(local_size_x = group_size, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D color_input; //calculate bloom
layout(rgba32f, binding = 1) uniform image2D blurred_hor1; //calculate bloom
layout(rgba32f, binding = 2) uniform image2D blurred_hor2; //calculate bloom
layout(rgba32f, binding = 3) uniform image2D DE_input; 
layout(rgba32f, binding = 4) uniform image2D DE_previous; //calculate final color

#include<utility/definitions.glsl>
#include<utility/uniforms.glsl>
#include<utility/camera.glsl>
#include<utility/shading.glsl>
#include<utility/bloom.glsl>

shared vec3 color_buffer[buffer_size];

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
	vec2 img_size = vec2(imageSize(blurred_hor1));
	
	vec2 res_ratio = vec2(imageSize(blurred_hor1))/ vec2(imageSize(color_input));
	
	//load a linear chunk of the image
	color_buffer[local_indx.x + group_size/2] = bloom_treshold(imageLoad(color_input, ivec2(vec2(global_pos)/res_ratio)).xyz, Camera.exposure);
	int new_coord = safe_coord(global_pos.x + shift(local_indx.x), img_size);
	color_buffer[local_indx.x + group_size/2 + shift(local_indx.x)] =
	(new_coord!=-1)?bloom_treshold(imageLoad(color_input, ivec2(vec2(new_coord,global_pos.y)/res_ratio)).xyz, Camera.exposure):vec3(0);
	memoryBarrierShared(); 
	barrier();
	
	float width = min(Camera.bloomradius*img_size.x/640, 128./15.);
	float k1 = 0.2;
	float k2 = 1;
	float a1 = 0.4;
	float a2 = 1;
	
	vec3 conv1 = vec3(0), conv2 = vec3(0);
	int center = local_indx.x+group_size/2;
	for(int i = max(center-int(width*15),0); i < min(center+int(width*15),buffer_size); i++)
	{
		float coord = float(center-i)/width;
		conv1 += exp(-k1*pow(coord,2))*color_buffer[i];
		conv2 += exp(-k2*pow(coord,2))*color_buffer[i];
	}
	
	imageStore(blurred_hor1, global_pos,  vec4(conv1,1));
	imageStore(blurred_hor2, global_pos,  vec4(conv2,1));
}