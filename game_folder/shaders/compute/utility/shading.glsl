#include<utility/ray_marching.glsl>
#include<utility/interpolation.glsl>
#include<utility/shading_functions.glsl>
#include<utility/random_stuff.glsl>

vec4 ambient_occlusion(in vec4 pos, in vec4 norm, in vec4 dir)
{	
	vec3 pos0 = pos.xyz;
	
	float occlusion_angle = 0.;
	vec3 direction = normalize(norm.xyz);
	vec3 ambient_color = ambient_sky_color(norm.xyz);
	//step out
	pos.xyz += 0.01*dir.w*direction;
	//march in the direction of the normal
	for(int i = 0; i < AMBIENT_MARCHES; i++)
	{
		pos.xyz += pos.w*direction;
		pos.w = DE(pos.xyz);
		
		norm.w = length(pos0 - pos.xyz);
		occlusion_angle += clamp(pos.w/norm.w,0.,1.);
	}
	
	occlusion_angle /= float(AMBIENT_MARCHES); // average weighted by importance
	return vec4(ambient_color,1.)*(0.5-cos(3.14159265*occlusion_angle)*0.5);
}

vec3 lighting(vec4 color, vec2 pbr, vec4 pos, vec4 dir, vec4 norm, vec3 refl, vec3 refr, vec3 direct, vec3 GI) 
{
	vec3 albedo = color.xyz;
	albedo = pow(albedo,vec3(1.f/gamma_material)); //square it to make the fractals more colorfull 
	
	vec4 ambient_color = ambient_occlusion(pos, norm, dir);
	
	GI *= ambient_color.w;
	float metallic = pbr.x;
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
	
	//reflectance equation
	vec3 Lo = vec3(0.0);
	vec3 V = -dir.xyz;
	vec3 N = norm.xyz;
	
	if(!SHADOWS_ENABLED)
	{
		direct = ambient_color.xyz;
	}

	{ //light contribution
		vec3 L = normalize(LIGHT_DIRECTION);
		vec3 radiance = direct;        

		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);                
		Lo += BRDF(V, L, N, albedo, pbr) * radiance * NdotL;
	}
	
	{ 
		//AO
		vec3 radiance = ambient_color.xyz * GI;               
		Lo += BRDF(V, N, N, albedo, pbr) * radiance;
	}
	
	return Lo;
}

vec3 shading_simple(in vec4 pos, in vec4 dir, float fov, vec3 direct, float k)
{
	if(pos.w < max(16*fovray*dir.w, MIN_DIST))
	{
		//calculate the normal
		float error = 0.5*fov*dir.w;
		vec4 norm = calcNormal(pos.xyz, max(MIN_DIST, error)); 
		norm.xyz = normalize(norm.xyz);
		if(norm.w < -error)
		{
			return COL(pos.xyz).xyz;
		}
		else
		{
			//optimize color sampling 
			vec3 cpos = pos.xyz - pos.w*norm.xyz;
			
			vec4 color; vec2 pbr; vec3 emission;
			scene_material(cpos, color, pbr, emission);
			return lighting(color, pbr, pos, dir, norm, vec3(0), vec3(0), direct, vec3(k)) + emission; 
		}
	}
	else
	{
		return sky_color(dir.xyz);
	}
}

vec3 render_ray(in vec4 pos, in vec4 dir, float fov, float k)
{
	vec4 var = vec4(0,0,0,1);
	ray_march(pos, dir, var, fov); 
	vec3 direct = sky_color(LIGHT_DIRECTION)*shadow_march(pos, vec4(LIGHT_DIRECTION,0), 10., LIGHT_ANGLE);
	return shading_simple(pos, dir, fov, direct, k);
}


vec3 marble_render(in vec4 pos, in vec4 dir, in vec4 norm, float fov, vec3 GI)
{
	vec3 Lo = vec3(0.);
	vec3 refl = vec3(0);
	vec3 refr = vec3(0);
	
	//Calculate refraction
	vec3 n = normalize(iMarblePos - pos.xyz);
	vec3 q = refraction(dir.xyz, n, 1.0 / 1.5);
	vec3 p2 = pos.xyz + (dot(q, n) * 2. * iMarbleRad) * q;
	n = normalize(p2 - iMarblePos);
	q = (dot(q, dir.xyz) * 2.0) * q - dir.xyz;
	vec4 p_temp = vec4(p2+ n*fov*dir.w*2.5, 0);
	vec4 r_temp = vec4(q, dir.w);
	
	refr = render_ray(p_temp, r_temp, fov*1.5, length(GI));

	//Calculate reflection
	n = -normalize(iMarblePos - pos.xyz);
	q = dir.xyz - n*(2*dot(dir.xyz,n));
	p_temp = vec4(pos.xyz + n*fov*dir.w*2., 0);
	r_temp = vec4(q, dir.w);
	
	refl = render_ray(p_temp, r_temp, fov*1.5, length(GI));
	vec3 V = -dir.xyz;
	//Combine for final marble color
	if(MARBLE_MODE == 0)
	{
		//glass
		vec3 F0 = vec3(0.08); 
		vec3 L = normalize(q);
		vec3 H = normalize(V + L);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);  
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		Lo = kS*refl + kD*refr;
	}
	else if(MARBLE_MODE > 0)
	{
		//metal
		vec3 F0 = vec3(0.6); 
		vec3 L = normalize(q);
		vec3 H = normalize(V + L);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);  
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		Lo = kS*refl;
	}
	
	return Lo;
}


vec3 shading(in vec4 pos, in vec4 dir, float fov, layout (rgba32f) image2D  illuminationDirect, vec3 RR)
{
	if(pos.w < max(2*fovray*dir.w, MIN_DIST))
	{
		//calculate the normal
		float error = 0.5*fov*dir.w;
		vec4 norm = calcNormal(pos.xyz, max(MIN_DIST, error)); 
		norm.xyz = normalize(norm.xyz);
		
		vec3 direct = bilinear_surface(illuminationDirect, dir.w, RR.z, RR.xy).xyz;
	
		if(norm.w < -error)
		{
			return COL(pos.xyz).xyz;
		}
		else
		{
			//optimize color sampling 
			vec3 cpos = pos.xyz - norm.w*norm.xyz;
			vec4 color; vec2 pbr; vec3 emission;
			scene_material(cpos, color, pbr, emission);
			vec3 refl = vec3(0);
			vec3 refr = vec3(0);
			if(color.w>0.5) // if marble
			{
				return marble_render(pos, dir, norm, fov, vec3(1.));
			}
			
			return lighting(color, pbr, vec4(cpos, pos.w), dir, norm, refl, refr, direct, vec3(1.)) + emission; 
		}
	}
	else
	{
		return sky_color(dir.xyz);
	}
	
}

#define NEON_iterations 3 
#define NEON_marches 4

vec3 NEON_shading(in vec4 pos, in vec4 dir)
{
	vec3 color = vec3(0);
	for (int i = 0; i < NEON_iterations; i++) 
	{	
		for (int j = 0; j < NEON_marches; j++) 
		{	
			dir.w += pos.w;
			pos.xyz += pos.w*dir.xyz;
			pos.w = DE(pos.xyz);
		}
		//sample color at the closest point
		vec4 norm = calcNormal(pos.xyz, MIN_DIST); 
		vec3 cpos = pos.xyz - pos.w*norm.xyz;
		
		color += COL(cpos).xyz/(1+pos.w);
	}
	return color/NEON_iterations;
}

vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return (x*(a*x+b))/(x*(c*x+d)+e);
}

vec3 HDRmapping(vec3 color, float exposure)
{
	// Exposure tone mapping
    vec3 mapped = ACESFilm(color * exposure);
    // Gamma correction 
    return pow(mapped, vec3(1.0 / gamma_camera));
}