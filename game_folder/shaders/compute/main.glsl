#version 430
//4*4 ray bundle
#define bundle_size 4
#define group_size 4
#define block_size 16
#define MAX_MARCHES 128
#include<camera.glsl>

//4*4 bundle of ray bundles(oh lol)
layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba8, binding = 0) uniform image2D img_output;

//make all the local ray bundles shared
shared ray ray_array[block_size][block_size]; 
//make the distance estimate data shared
shared float DE_data[block_size][block_size]; 

#include<ray_marching.glsl>

//each shader invocation will march a ray bundle,
//all invocations will have all the DE data from neighbore
//ray bundles to improve marching efficiency
//since it will give a larger volume of space in which the rays can safely march
//(not only a single DE sphere)

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.x,gl_GlobalInvocationID.y);
	ivec2 local_pos = ivec2(gl_LocalInvocationID.x,gl_LocalInvocationID.y);

	//initialize the ray bundle
	for(int i = 0; i < bundle_size; i++)
	{
		for(int j = 0; j < bundle_size; j++)
		{
			ivec2 lpos = bundle_size*local_pos + ivec2(i,j);
			ivec2 pos = bundle_size*global_pos + ivec2(i,j);
			ray_array[lpos.x][lpos.y] = get_ray(vec2(pos)/vec2(imageSize(img_output)));
			DE_data[lpos.x][lpos.y] = 0;
		}	  
	}

	//wait untill all the workgroup data is written
	memoryBarrierShared();
	
	bool complete[bundle_size][bundle_size];
	for(int i = 0; i < bundle_size; i++)
	{
		for(int j = 0; j < bundle_size; j++)
		{
			complete[i][j] = false;
		}	  
	}
	
	bool bundle_computing = true;
	
	//central ray position
	int crx = bundle_size/2, cry = bundle_size/2;
	
	int n = 0;
	///Ray bundle marching
	while(bundle_computing && n<MAX_MARCHES)
	{
		n++;
		bundle_computing = false;
		//march the central rays
		if(!complete[crx][cry])
		{
			ivec2 lpos = bundle_size*local_pos + ivec2(crx,cry);
			complete[crx][cry] = march_ray_bundle(lpos);
			bundle_computing = bundle_computing || !complete[crx][cry];
		}
		
		//march the secondary rays
		for(int i = 0; i < bundle_size; i++)
		{
			for(int j = 0; j < bundle_size; j++)
			{
				if(i != crx && j != cry && !complete[i][j])
				{
					ivec2 lpos = bundle_size*local_pos + ivec2(i,j);
					complete[i][j] = march_ray_bundle(lpos);
					bundle_computing = bundle_computing && !complete[i][j];
				}
			}	  
		}
	}
	
	
	/*for(int i = 0; i < bundle_size; i++)
	{
		for(int j = 0; j < bundle_size; j++)
		{
			ivec2 lpos = bundle_size*local_pos + ivec2(i,j);
			ray_march(ray_array[lpos.x][lpos.y]);
		}	  
	}*/
	
	vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);
	
	// output to the specified image block
	for(int i = 0; i < bundle_size; i++)
	{
		for(int j = 0; j < bundle_size; j++)
		{
			//render the depth map
			ivec2 lpos = bundle_size*local_pos + ivec2(i,j);
			ivec2 pos = bundle_size*global_pos + ivec2(i,j);
			float td = ray_array[lpos.x][lpos.y].td;
			vec3 depth = vec3(td,td,td)/10.f;
			imageStore(img_output, pos, vec4(depth,1));
		}	  
	}
}