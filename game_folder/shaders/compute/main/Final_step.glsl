#version 430
//4*4 ray bundle
#define group_size 8
#define buffer_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D bloom; 
layout(rgba8, binding = 1) uniform image2D final_color; 
layout(rgba32f, binding = 2) uniform image2D DE_input; 
layout(rgba32f, binding = 3) uniform image2D color_HDR0; 
layout(rgba32f, binding = 4) uniform image2D prevDE; 
layout(rgba32f, binding = 5) uniform image2D color_HDR1; 

//??
shared vec4 de_sph[1][1]; 

#include<utility/camera.glsl>
#include<utility/shading.glsl>

#define DOF_samples 100
#define DOF

#define RA 10.
const float GA = 2.399; 
const mat2 rot = mat2(cos(GA),sin(GA),-sin(GA),cos(GA));

//DOF radius camera distance dependence
float radius(float w)
{
    return (Camera.bokeh+0.001)*(max(w-Camera.focus,0.001)+max(Camera.size-w,1e-3))/(abs(w)+1e-3); // the angle 
}

float convol_kernel(float w1, float w2, float rad)
{
    float r1 = radius(w1);
    float r2 = radius(w2);
    float cent_p = step((rad-1.),r1);
    float this_p = step((rad-1.),r2);
    return pow(r2,-0.5)*(this_p*cent_p+step(r1,r2)*step(w2,w1)*this_p);
}

vec3 dof(vec2 uv)
{
    float rad =RA/sqrt(DOF_samples);
	vec2 img_size = vec2(imageSize(color_HDR1));
    vec2 pixel=vec2(img_size.x)*0.001, angle=vec2(0,rad);
    vec4 acc=vec4(0);
    vec4 center = imageLoad(color_HDR1, ivec2(uv+pixel*(rad-1.)*angle));
    acc += convol_kernel(center.w, center.w, rad)*vec4(center.xyz,1.);
    rad=1.;
	if(Camera.bokeh > 0.1) for (int j=0;j<DOF_samples;j++)
    {
        rad += 1./rad;
	    angle *= rot;
        
        vec4 col = imageLoad(color_HDR1, ivec2(uv+pixel*(rad-1.)*angle));

        acc += convol_kernel(center.w, col.w, rad)*vec4(col.xyz,1.);  
    }
	return acc.xyz/acc.w;
}

#define VIGNETTE_STRENGTH 0.2

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	vec2 img_size = vec2(imageSize(color_HDR1));
	
	vec2 res_ratio = vec2(imageSize(bloom))/img_size;
	vec3 bloom_color = interp(bloom, vec2(global_pos)*res_ratio).xyz;
	
	imageStore(color_HDR0, global_pos, imageLoad(color_HDR1, global_pos));
	#ifdef DOF
		vec3 fin_color = dof(vec2(global_pos)) + bloom_color;
	#else
		vec3 fin_color = imageLoad(color_HDR1, global_pos).xyz + bloom_color;
	#endif
	
	float vignette = 1.0 - VIGNETTE_STRENGTH * length(vec2(global_pos)/img_size - 0.5);
	imageStore(final_color, global_pos, vec4(HDRmapping(fin_color, Camera.exposure)*vignette, 1));	 
}