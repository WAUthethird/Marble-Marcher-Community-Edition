# Marble Marcher: CE shader pipeline documentation

*Note: compared to other games MMCE uses only compute shaders to render the image, since they allow a higher degree of flexibility while dealing with the GPU.*

## Simple shader configuration

MMCE has a shader configuration system, each shader configuration is written in a .cfg file in the compute folder.

A simple example of a shader configuration is this:

```
#number of main textures
2
#####################################################
#shader name
noise/noise.glsl
#global size
width/8
height/8
#texture resolution
width
height
#output texture number
1
#####################################################
```

Each comfiguration defines a shader pipeline, a set of shaders executed in order.

All the lines starting with a '#' are ignored, and can be used as comments.
The first uncommented line describes the number of global images, which are available in every shader in the configuration, and can be written or read at the same time, OpenGL flag - READ_WRITE.
The next line locates the shader code file(on shaders later).
The next 2 lines define the number of workgroups the shader invocation will have. 
And the 2 lines after define the resolution of the texture used for this shader invocation. 
The last line defines the number of textures this shader stage will write to, you can not read from them, they are WRITE_ONLY.
The lines with numbers can hold any expression, and it will be calculated after the shader confing is loaded. there are predefined constants available:
- `width` and `height` hold the current rendering resolution chosen by the user.
- `MRRM_scale` holds the downscaling factor of the first stage of Multi-res marching, aka Cone marching.
- `shadow_scale` the downscaling factor of shadows
- And so on, you can in practice add more variables by editing 'Renderer.cpp'

*Note: compute shaders have a limit of 8 images for a single shader, meaning the sum of global and local images, the texture samplers are not counted*

Now lets look at the shader itself:

```glsl
#version 430
#define group_size 8

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba8, binding = 0) uniform image2D final_color; //final output texture 1 (used as final color)
//global, always the last ones
layout(rgba32f, binding = 1) uniform image2D DE_input; //global tex
layout(rgba32f, binding = 2) uniform image2D color_HDR; //global tex

#define PI 3.14159265
#include<utility/RNG.glsl>

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	vec2 img_size = vec2(imageSize(color_HDR));
	
	vec2 uv = vec2(global_pos)/img_size;
	
	//vec3 col = 0.5*perlin4(25*uv).xyz + 0.5;
	vec3 col = blue3(300.*uv, float(iFrame)*0.2);
	imageStore(final_color, global_pos, vec4(col, 1));	 
}
```

Compute shaders work by invoking so called work-groups, groups of GPU cores that have shared memory. The size of the work group is defined by `layout(local_size_x = group_size, local_size_y = group_size) in;`. The size of the workgroup can be whatever, but 8 by 8 is the default. 
It is recommended that the total number of GPU cores in a single workgroup is at least 64, otherwise you may get a slowdown. 
The size of the work-group is what explains why the number of work-groups is divided by 8, or any other number. The total number of GPU core invocations needs to be equal or more than the rendering resolution you desire. 
To be precise the number of workgroups is exactly equal to `ceil(\expression in cfg\)`. So the number of GPU core invocations is exactly the number of workgroups times the number of GPU cores in a work-group.

The next important point is the binding order of the images. In this simple example you only have the shader output image and the global images. In cases where the config has more than 1 shader you can only access the image output of the previous shader.
The exact order of images is as follows:
- First are the output images of the previous shader, if the number of previous images is zero then you skip this.
- Next are the output images of this shader, in this case there is 1 image.
- And last are the global images

*Note: the FIRST output of the LAST shader has a type rgba8(output range from 0 to 1, 8 bit precision) everything else is in floats rgba32f*

As you can see from the code you can include code from other files, MMCE just loads them and inserts the code, so its a fake include, in fact.
If the shader compilation produces errors MMCE will make an 'name'-error.txt file with the full code and the errors at the end.

In the shader code itself you can write and read from the images in any order by using 
- `imageLoad(image2D imgname, ivec2 pixel)`
- `imageStore(image2D imgname, ivec2 pixel, vec4 stored_value)`

## Main shader configuration 

Lets look at what each shader step does in this configuration.

```
#############MAIN SHADER PIPELINE###################
#number of main textures(1 depth map, 2 HDR color) 
#this should be first, at least 2 textures
4
####################################################
#shader name 
multires_marching/MRRM1.glsl
#global work-group number
width/(8*MRRM_scale)
height/(8*MRRM_scale)
#texture resolution
width/MRRM_scale
height/MRRM_scale
#output texture number
3
####################################################
#shader name
multires_marching/MRRM2.glsl
#global work-group number
width/8
height/8
#texture resolution
width
height
#output texture number
0
####################################################
#shader name #half res shadows and AO
main/Illumination_step.glsl
#global work-group number
width/(shadow_scale*8)
height/(shadow_scale*8)
#texture resolution
width/shadow_scale
height/shadow_scale
#output texture number
1
####################################################
#shader name
main/Shading_step.glsl
#global work-group number
width/8
height/8
#texture resolution
width
height
#output texture number
1
####################################################
#shader name TEMPORAL ANTI-ALIASING
post_processing/Temporal_Denoiser.glsl
#global work-group number
width/8
height/8
#texture resolution
width
height
#output texture number
1
####################################################
#shader name
post_processing/downsampling.glsl
#global work-group number
width/(8*bloom_scale)
height/(8*bloom_scale)
#texture resolution
width/bloom_scale
height/bloom_scale
#output texture number
1
####################################################
#shader name
post_processing/Bloom_horiz.glsl
#global work-group number
width/(bloom_scale*128)
height/bloom_scale
#texture resolution
width/bloom_scale
height/bloom_scale
#output texture number
2
####################################################
#shader name
post_processing/Bloom_vertic.glsl
#global work-group number
width/bloom_scale
height/(bloom_scale*128)
#texture resolution
width/bloom_scale
height/bloom_scale
#output texture number
1
####################################################
#shader name
main/Final_step.glsl
#global work-group number
width/8
height/8
#texture resolution
width
height
#output texture number
1
####################################################
```

There are 4 global images, the fist one is the positions of the ray-marched points with the distace estimator value like this `vec4(pos, DE)`, second one stores the previous frame points, third one stores the normals, and the forth is for HDR linear space colors.
 
The pipeline:
- The first 2 shaders are the multi-res ray-marching, only 2 steps. Is is also known as cone marching http://www.fulcrum-demo.org/wp-content/uploads/2012/04/Cone_Marching_Mandelbox_by_Seven_Fulcrum_LongVersion.pdf
- The Illumination_step shader computes soft ray-marched shadows.
- The Shading_step shader paints the fractal surface using a PBR BRDF using the previously computed shadows, plus marched AO
- The Temporal_Denoiser shader uses the previous positions and the previous camera to reproject the previous frame color and blend it with the current one to remove noise, it is a custom implementation of TXAA.
- The downsampling shader does what it is called
- Bloom_horiz and Bloom_vertic are a 2 pass gaussian blur filter wich uses compute shader shared memory magic to accelerate the memory access, that explains the weird work-group sizes.
- The last shader is mainly just post-processing and DOF.
