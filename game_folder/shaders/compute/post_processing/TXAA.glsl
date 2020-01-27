#version 430
//4*4 ray bundle
#define group_size 8
#define buffer_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;
layout(rgba32f, binding = 0) uniform image2D color_input; 
layout(rgba32f, binding = 1) uniform image2D color_output; 
layout(rgba32f, binding = 2) uniform image2D DE_input; 
layout(rgba32f, binding = 3) uniform image2D color_HDR0; //prev color
layout(rgba32f, binding = 4) uniform image2D prevDE; //calculate final color
layout(rgba32f, binding = 5) uniform image2D color_HDR1; //final color

// Temporal AA based on Epic Games' implementation:
// https://de45xmedrsdbp.cloudfront.net/Resources/files/TemporalAA_small-59732822.pdf
// 
// Originally written by yvt for https://www.shadertoy.com/view/4tcXD2
// Feel free to use this in your shader!

// YUV-RGB conversion routine from Hyper3D
vec3 encodePalYuv(vec3 rgb)
{
    rgb = rgb; // gamma correction
    return vec3(
        dot(rgb, vec3(0.299, 0.587, 0.114)),
        dot(rgb, vec3(-0.14713, -0.28886, 0.436)),
        dot(rgb, vec3(0.615, -0.51499, -0.10001))
    );
}

vec3 decodePalYuv(vec3 yuv)
{
    vec3 rgb = vec3(
        dot(yuv, vec3(1., 0., 1.13983)),
        dot(yuv, vec3(1., -0.39465, -0.58060)),
        dot(yuv, vec3(1., 2.03211, 0.))
    );
    return rgb; // gamma correction
}

//make all the local distance estimator spheres shared
shared vec4 de_sph[group_size][group_size]; 

#include<utility/camera.glsl>
#include<utility/shading.glsl>

#define blur 0.05

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	
	vec2 img_size = vec2(imageSize(color_input));
	vec2 res_ratio = vec2(imageSize(DE_input))/img_size;
	
	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	vec4 var = vec4(0);
	
	//TXAA
	vec4 sph = imageLoad(DE_input, global_pos);
	float td = length(sph.xyz - pos.xyz);//traveled distance
	
    //getting the previous frame pixel and sampling it bicubically 
	vec2 lastCoord = reproject(pos.xyz + td*dir.xyz, vec2(global_pos)/img_size);
    vec4 lastColor = interp_bicubic(color_HDR0, lastCoord);
	
	vec3 antialiased = vec3(0.);
	antialiased = lastColor.xyz;
	
    float mixRate = min(lastColor.w, 0.6);
    
    ivec3 off = ivec3(-1, 0, 1);
    vec3 in0 = imageLoad(color_input, global_pos).xyz;
    antialiased = mix(antialiased * antialiased, in0 * in0, mixRate);
    antialiased = sqrt(antialiased);
    
    vec3 in1 = imageLoad(color_input, global_pos + off.zy).xyz;
    vec3 in2 = imageLoad(color_input, global_pos + off.xy).xyz;
    vec3 in3 = imageLoad(color_input, global_pos + off.yz).xyz;
    vec3 in4 = imageLoad(color_input, global_pos + off.yx).xyz;
    vec3 in5 = imageLoad(color_input, global_pos + off.zz).xyz;
    vec3 in6 = imageLoad(color_input, global_pos + off.xz).xyz;
    vec3 in7 = imageLoad(color_input, global_pos + off.zx).xyz;
    vec3 in8 = imageLoad(color_input, global_pos + off.xx).xyz;
    
    antialiased = encodePalYuv(antialiased);
    in0 = encodePalYuv(in0);
    in1 = encodePalYuv(in1);
    in2 = encodePalYuv(in2);
    in3 = encodePalYuv(in3);
    in4 = encodePalYuv(in4);
    in5 = encodePalYuv(in5);
    in6 = encodePalYuv(in6);
    in7 = encodePalYuv(in7);
    in8 = encodePalYuv(in8);
    
    vec3 minColor = min(min(min(in0, in1), min(in2, in3)), in4);
    vec3 maxColor = max(max(max(in0, in1), max(in2, in3)), in4);
    minColor = mix(minColor,
       min(min(min(in5, in6), min(in7, in8)), minColor), 0.5);
    maxColor = mix(maxColor,
       max(max(max(in5, in6), max(in7, in8)), maxColor), 0.5);
    
   	vec3 preclamping = antialiased;
	antialiased.x = clamp(antialiased.x, minColor.x*(1. - blur), maxColor.x*(1.+blur));
    antialiased.yz = clamp(antialiased.yz, minColor.yz, maxColor.yz);
    
    mixRate = 1.0 / (1.0 / mixRate + 1.0);
    
    vec3 diff = antialiased - preclamping;
    float clampAmount = dot(diff, diff);
    
    mixRate += clampAmount * 4.0;
    mixRate = clamp(mixRate, 0.05, 0.5);
    
    antialiased = decodePalYuv(antialiased);
	imageStore(color_output, global_pos, vec4(antialiased, td));
	imageStore(color_HDR1, global_pos, vec4(antialiased, mixRate));
}