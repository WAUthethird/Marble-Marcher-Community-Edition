#include<ray_marching.glsl>

#define PI 3.14159265
#define AMBIENT_MARCHES 5
#define AMBIENT_COLOR 2*vec4(1,1,1,1)
#define LIGHT_ANGLE 0.08

uniform vec3 BACKGROUND_COLOR;
uniform vec3 LIGHT_DIRECTION;
uniform float PBR_METALLIC;
uniform float PBR_ROUGHNESS;
uniform vec3 LIGHT_COLOR;
uniform bool SHADOWS_ENABLED; 

//better to use a sampler though
vec4 interp(layout (rgba32f) image2D text, vec2 coord)
{
	//coord *= 0.99;
	ivec2 ci = ivec2(coord);
	vec2 d = coord - floor(coord);
	return imageLoad(text, ci)*(1-d.x)*(1-d.y) +
		   imageLoad(text, ci+ivec2(1,0))*d.x*(1-d.y) +
		   imageLoad(text, ci+ivec2(0,1))*(1-d.x)*d.y +
		   imageLoad(text, ci+ivec2(1))*d.x*d.y;
}


///PBR functions 
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
///END PBR functions

float ambient_occlusion(in vec4 pos, in vec4 norm, in vec4 dir)
{
	vec3 dir1 = normalize(cross(dir.xyz,norm.xyz));
	vec3 dir2 = normalize(cross(dir1,norm.xyz));
	pos.w = iMarbleRad/2; 
	
	vec3 pos0 = pos.xyz;
	
	float shifter = 2;
	float dcoef = 0.02/iMarbleRad;
	float occlusion_angle = 0;
	float integral = 0;
	float i_coef = 0;
	
	//march in the direction of the normal
	#pragma unroll
	for(int i = 0; i < AMBIENT_MARCHES; i++)
	{
		//moving in a zig-zag
		vec3 direction = normalize(norm.xyz + 0.25*((mod(shifter,3.f)-1)*dir1 +  (mod(shifter+1,3.f)-1)*dir2));
		pos.xyz += pos.w*direction;
		pos.w = DE(pos.xyz);
		
		norm.w = length(pos0 - pos.xyz);
		i_coef = 1/(dcoef*norm.w+1);//importance
		occlusion_angle += i_coef*clamp(pos.w/norm.w,0,1);
		integral += i_coef;
	}
	
	occlusion_angle /= integral; // average weighted by importance
	
	return 0.5-cos(3.14159265*occlusion_angle)*0.5;
}

const float Br = 0.0025;
const float Bm = 0.0003;
const float g =  0.9800;
const vec3 nitrogen = vec3(0.650, 0.570, 0.475);
const vec3 Kr = Br / pow(nitrogen, vec3(4.0));
const vec3 Km = Bm / pow(nitrogen, vec3(0.84));

vec3 sky_color(in vec3 pos)
{
	// Atmosphere Scattering
	vec3 fsun = LIGHT_DIRECTION;
	float brightnees = exp(min(5*pos.y,0));
	if(pos.y < 0)
	{
		pos.y = 0; 
		pos.xyz = normalize(pos.xyz);
	}
    float mu = dot(normalize(pos), normalize(fsun));
	
	vec3 extinction = mix(exp(-exp(-((pos.y + fsun.y * 4.0) * (exp(-pos.y * 16.0) + 0.1) / 80.0) / Br) * (exp(-pos.y * 16.0) + 0.1) * Kr / Br) * exp(-pos.y * exp(-pos.y * 8.0 ) * 4.0) * exp(-pos.y * 2.0) * 4.0, vec3(1.0 - exp(fsun.y)) * 0.2, -fsun.y * 0.2 + 0.5);
	vec3 sky_col = brightnees* 3.0 / (8.0 * 3.14) * (1.0 + mu * mu) * (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm) * extinction;
	return 0.4*clamp(sky_col,0,10);
}

vec3 refraction(vec3 rd, vec3 n, float p) {
	float dot_nd = dot(rd, n);
	return p * (rd - dot_nd * n) + sqrt(1.0 - (p * p) * (1.0 - dot_nd * dot_nd)) * n;
}

/*
void refraction_marble(vec3 p, vec3 r)
{
	vec3 n = normalize(iMarblePos - p.xyz);
	vec3 q = refraction(r, n, 1.0 / 1.5);
	vec3 p2 = p.xyz + (dot(q, n) * 2.0 * iMarbleRad) * q;
	n = normalize(p2 - iMarblePos);
	q = (dot(q, r) * 2.0) * q - r;
	vec4 p_temp = vec4(p2 + n * (MIN_DIST * 10), 1.0);
	vec4 r_temp = vec4(q, 0.0);
}
*/

vec4 lighting(vec4 pos, vec4 dir, vec4 norm, vec3 reflection, vec3 refraction, float shadow) 
{
	//optimize color sampling 
	vec3 cpos = pos.xyz - norm.w*norm.xyz;
	cpos = cpos - DE(cpos)*norm.xyz;
	cpos = cpos - DE(cpos)*norm.xyz;
	vec3 albedo = COL(cpos).xyz;
	albedo *= albedo;
	
	
	float ao = ambient_occlusion(pos, norm, dir);
	vec4 ambient_color = vec4(BACKGROUND_COLOR,1)*ao;
	
	float metallic = PBR_METALLIC;
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
	
	//reflectance equation
	vec3 Lo = vec3(0.0);
	vec3 V = -dir.xyz;
	vec3 N = norm.xyz;
	
	{ //ambient occlusion contribution
		float roughness = max(PBR_ROUGHNESS,0.5);
		vec3 L = normalize(N);
		vec3 H = normalize(V + L);
		vec3 radiance = ambient_color.xyz;        
		
		// cook-torrance brdf
		float NDF = DistributionGGX(N, H, roughness);        
		float G   = GeometrySmith(N, V, L, roughness);      
		vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;	  
		
		vec3 numerator    = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular     = numerator / max(denominator, 0.001);  
			
		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);                
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
	
	if(!SHADOWS_ENABLED)
	{
		shadow = ao;
	}
	
	vec3 light_color = sky_color(LIGHT_DIRECTION);
	
	{ //light contribution
			float roughness = PBR_ROUGHNESS;
			vec3 L = normalize(LIGHT_DIRECTION);
			vec3 H = normalize(V + L);
			vec3 radiance = light_color*shadow*(0.6+0.4*ao);        
			
			// cook-torrance brdf
			float NDF = DistributionGGX(N, H, roughness);        
			float G   = GeometrySmith(N, V, L, roughness);      
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
			
			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - metallic;	  
			
			vec3 numerator    = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
			vec3 specular     = numerator / max(denominator, 0.001);  
				
			// add to outgoing radiance Lo
			float NdotL = max(dot(N, L), 0.0);                
			Lo += (kD * albedo / PI + specular) * radiance * NdotL;
		}
		
		{ //light reflection, GI imitation
			float roughness = max(PBR_ROUGHNESS,0.5);
			vec3 L = normalize(-LIGHT_DIRECTION);
			vec3 H = normalize(V + L);
			vec3 radiance = 0.5*light_color*ao*(1-ao);        
			
			// cook-torrance brdf
			float NDF = DistributionGGX(N, H, roughness);        
			float G   = GeometrySmith(N, V, L, roughness);      
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
			
			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - metallic;	  
			
			vec3 numerator    = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
			vec3 specular     = numerator / max(denominator, 0.001);  
				
			// add to outgoing radiance Lo
			float NdotL = max(dot(N, L), 0.0);                
			Lo += (kD * albedo / PI + specular) * radiance * NdotL;
		}
	
	return vec4(Lo,1);
}

vec4 shading(in vec4 pos, in vec4 dir, float fov, float shadow)
{
	//calculate the normal
	float error = fov*dir.w;
	vec4 norm = calcNormal(pos.xyz, error/2); 
	norm.xyz = normalize(norm.xyz);

	return lighting(pos, dir, norm, vec3(0), vec3(0), shadow); 
}

/*
vec3 render_ray(in vec4 pos, in vec4 dir, float fov)
{
	vec4 var = vec4(0);
	ray_march(pos, dir, var, fov, MIN_DIST); 
	float shadow = shadow_march(pos, LIGHT_DIRECTION, MAX_DIST, LIGHT_ANGLE);
	shading(pos, dir, fov, shadow);
	
	
}
*/

vec3 HDRmapping(vec3 color, float exposure, float gamma)
{
	// Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-color * exposure);
    // Gamma correction 
    return pow(mapped, vec3(1.0 / gamma));
}