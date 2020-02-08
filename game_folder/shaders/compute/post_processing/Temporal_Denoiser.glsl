#include<utility/compute_shader_header.glsl>

layout(rgba32f, binding = 0) uniform image2D color_input; 
layout(rgba32f, binding = 1) uniform image2D color_output; 
layout(rgba32f, binding = 2) uniform image2D DE_input; 
layout(rgba32f, binding = 3) uniform image2D DE_previous; //calculate final color
layout(rgba32f, binding = 4) uniform image2D normals; //final color
layout(rgba32f, binding = 5) uniform image2D HDR0;  
layout(rgba32f, binding = 6) uniform image2D HDR1; 
layout(rgba32f, binding = 7) uniform image2D GI; 

// Temporal Denoiser With Reprojection

#include<utility/definitions.glsl>
#include<utility/uniforms.glsl>
#include<utility/camera.glsl>
#include<utility/shading.glsl>

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

void main() {
	ivec2 global_pos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 local_indx = ivec2(gl_LocalInvocationID.xy);
	
	vec2 img_size = vec2(imageSize(color_input));
	vec2 res_ratio = vec2(imageSize(DE_input))/img_size;
	res_ratio = vec2(res_ratio.x, round(res_ratio.y)); 
	
	ray rr = get_ray(vec2(global_pos)/img_size);
	vec4 pos = vec4(rr.pos,0);
	vec4 dir = vec4(rr.dir,0);
	vec4 var = vec4(0);
	
	vec4 sph = imageLoad(DE_input, ivec2((global_pos)*res_ratio));
	float td = length(sph.xyz - pos.xyz);//traveled distance
	pos.xyz = pos.xyz + td*dir.xyz;
    //getting the previous frame pixel and sampling it bicubically 
	vec2 lastCoord = reproject(pos.xyz, (vec2(global_pos))/img_size);
	
    vec4 lastColor = interp_bicubic(HDR0, clamp(lastCoord/res_ratio,vec2(2.),img_size-2.));
	vec4 lastPos = interp(DE_previous, round(lastCoord));
	
	ray pr = get_ray(PrevCamera, clamp(lastCoord,vec2(0.),imageSize(DE_input)-1.)/imageSize(DE_input));
	vec4 ppos = vec4(pr.pos,0);
	vec4 pdir = vec4(pr.dir,0);
	float tdprev = length(lastPos.xyz - ppos.xyz);//traveled distance
	lastPos.xyz = ppos.xyz + tdprev*pdir.xyz;
	
    vec3 in0 = imageLoad(color_input, global_pos).xyz;
	
	////rejecting some of the previous data
	//the previous point UV coordinate in the current camera plane
	vec2 lastUV = project(lastPos.xyz, vec2(global_pos)/img_size);
	float delta = length(lastUV - global_pos) + 3.*sqrt(abs(dot(dir.xyz, pr.pos.xyz - rr.pos.xyz))/td);
	
	//remove prev data based on color difference
	ivec3 off = ivec3(-1, 0, 1);
	vec3 in1 = imageLoad(color_input, global_pos + off.zy).xyz;
    vec3 in2 = imageLoad(color_input, global_pos + off.xy).xyz;
    vec3 in3 = imageLoad(color_input, global_pos + off.yz).xyz;
    vec3 in4 = imageLoad(color_input, global_pos + off.yx).xyz;
    vec3 in5 = imageLoad(color_input, global_pos + off.zz).xyz;
    vec3 in6 = imageLoad(color_input, global_pos + off.xz).xyz;
    vec3 in7 = imageLoad(color_input, global_pos + off.zx).xyz;
    vec3 in8 = imageLoad(color_input, global_pos + off.xx).xyz;
	
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
	vec3 avgColor = (in0 + in1 + in2 + in3 + in4 + in5 + in6 + in7 + in8)/9.;
	vec3 colorRange = abs(maxColor - minColor) + 0.001;
	vec3 dColor = (encodePalYuv(lastColor.xyz/lastColor.w) - avgColor)/colorRange;
	lastColor *= 0.15 + 0.85*exp(-TXAAstr*dot(dColor,dColor));
	
	//remove prev data based on relative pixel distance
	lastColor *= exp(-pow(delta, 2.));
	
	//remove prev data based on camera shift
	//lastColor *= (0.95 + 0.03*tanh(4. - length( global_pos*res_ratio - lastCoord )));
	
	
	if(iFrame < 1 || isnan(lastColor.x)) lastColor = vec4(0.);
	
	//adding new pixel
	lastColor.xyz += decodePalYuv(in0);
	lastColor.w += 1.;
	
	if(!isnan(lastColor.x))
	{
		imageStore(color_output, global_pos, vec4(lastColor.xyz/lastColor.w, td));
		imageStore(HDR1, global_pos, vec4(lastColor));
	}
}