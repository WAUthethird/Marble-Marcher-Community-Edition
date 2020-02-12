# Marble Marcher: CE shader pipeline documentation

*Note: compared to other games MMCE uses only compute shaders to render the image, since they allow a higher degree of flexibility while dealing with the GPU.*

##Simple shader configuration

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
The last line difines the number of textures this shader stage will write to, you can not read from them, they are WRITE_ONLY.
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

Compute shaders work by invoking so called work-groups, groups of GPU cores that have shared memory. The size of the work group is defined by `layout(local_size_x = group_size, local_size_y = group_size) in;`. The size of the workgroup can be whatever, but 8 by 8 is the default. It is recommended that the total number of GPU cores in a single workgroup is at least 64, otherwise you may get a slowdown. 
The size of the work-group is what expains why the number of work-groups is divided by 8, or any other number. The total number of GPU core invocations needs to be equal or more than the rendering resolution you desire. To be precise the number of workgroups is exactly equal to `ceil(\expression in cfg\)`. So the number of GPU core invocations is exactly the number of workgroups times the number of GPU cores in a work-group.



