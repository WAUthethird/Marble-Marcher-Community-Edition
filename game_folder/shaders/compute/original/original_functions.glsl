#define DIFFUSE_ENABLED 0
#define DIFFUSE_ENHANCED_ENABLED 1
#define FILTERING_ENABLE 0
#define FOCAL_DIST 1.73205080757
#define FOG_ENABLED 0
#define SHADOW_DARKNESS 0.7
#define SHADOW_SHARPNESS 10.0
#define SPECULAR_HIGHLIGHT 40
#define SPECULAR_MULT 0.25
#define SUN_ENABLED 1
#define SUN_SHARPNESS 2.0
#define SUN_SIZE 0.004
#define BACKGROUND_COLOR vec3(0.6,0.8,1.0)


vec3 sky_color_orig(vec3 dir)
{
	vec3 col = BACKGROUND_COLOR;

	//Background specular
	#if SUN_ENABLED
		float sun_spec = dot(dir, LIGHT_DIRECTION) - 1.0 + SUN_SIZE;
		sun_spec = min(exp(sun_spec * SUN_SHARPNESS / SUN_SIZE), 1.0);
		col.xyz += LIGHT_COLOR * sun_spec;
	#endif
	
	return col;
}

vec3 lighting_original(vec4 color, vec2 pbr, vec4 pos, vec4 dir, vec4 norm, vec3 refl, vec3 refr, float shadow) 
{
	color.xyz *= 2.;
	vec3 col = vec3(0.);
	//Get if this point is in shadow
	float k = shadow;
	vec4 ambient_color = ambient_occlusion(pos, norm, dir);
	if(!SHADOWS_ENABLED)
	{
		k = ambient_color.w;
	}
	vec3 n = norm.xyz;
	//Get specular
	#if SPECULAR_HIGHLIGHT > 0
		vec3 reflected = dir.xyz - 2.0*dot(dir.xyz, n) * n;
		float specular = max(dot(reflected, LIGHT_DIRECTION), 0.0);
		specular = pow(specular, SPECULAR_HIGHLIGHT);
		col.xyz += specular * LIGHT_COLOR * (k * SPECULAR_MULT);
	#endif

	//Get diffuse lighting
	#if DIFFUSE_ENHANCED_ENABLED
		k = min(k, SHADOW_DARKNESS * 0.5 * (dot(n, LIGHT_DIRECTION) - 1.0) + 1.0);
	#elif DIFFUSE_ENABLED
		k = min(k, dot(n, LIGHT_DIRECTION));
	#endif

	//Don't make shadows entirely dark
	k = max(k, 1.0 - SHADOW_DARKNESS);
	col.xyz += color.xyz * LIGHT_COLOR * k;

	//Add small amount of ambient occlusion
	col.xyz *= 0.5 + 0.5*ambient_color.w;

	//Add fog effects
	#if FOG_ENABLED
		float b = dir.w / MAX_DIST;
		col.xyz = (1.0 - b) * col.xyz + b * BACKGROUND_COLOR;
	#endif
	
	vec3 V = -dir.xyz;
	vec3 N = norm.xyz;
	
	if(color.w>0.5) // if marble
	{
		col *= 0.;
		vec3 n = norm.xyz;
		vec3 q = dir.xyz - n*(2*dot(dir.xyz,n));
		//Combine for final marble color
		if(MARBLE_MODE == 0)
		{
			//glass
			vec3 F0 = vec3(0.03); 
			vec3 L = normalize(q);
			vec3 H = normalize(V + L);
			vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);  
			
			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			col += kS*refl + kD*refr;
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
			col += kS*refl;
		}
	}
	
	return col;
}



vec3 shading_simple_orig(in vec4 pos, in vec4 dir, float fov, float shadow)
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
			//cpos = cpos - DE(cpos)*norm.xyz;
			//cpos = cpos - DE(cpos)*norm.xyz;
			
			vec4 color; vec2 pbr; vec3 emission;
			scene_material(cpos, color, pbr, emission);
			return lighting_original(color, pbr, pos, dir, norm, vec3(0), vec3(0), shadow); 
		}
	}
	else
	{
		return sky_color_orig(dir.xyz);
	}
}


vec3 render_ray_orig(in vec4 pos, in vec4 dir, float fov)
{
	vec4 var = vec4(0,0,0,1);
	ray_march(pos, dir, var, fov); 
	float shadow = shadow_march(pos, vec4(LIGHT_DIRECTION,0), 10., LIGHT_ANGLE);
	return shading_simple_orig(pos, dir, fov, shadow);
}

vec3 shading_orig(in vec4 pos, in vec4 dir, float fov, float shadow)
{
	if(pos.w < max(2*fovray*dir.w, MIN_DIST))
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
			vec3 cpos = pos.xyz - norm.w*norm.xyz;
			vec4 color; vec2 pbr; vec3 emission;
			scene_material(cpos, color, pbr, emission);
			vec3 refl = vec3(0);
			vec3 refr = vec3(0);
			if(color.w>0.5) // if marble
			{
				vec3 n = normalize(iMarblePos - cpos.xyz);
				vec3 q = refraction(dir.xyz, n, 1.0 / 1.5);
				vec3 p2 = pos.xyz + (dot(q, n) * 2. * iMarbleRad) * q;
				n = normalize(p2 - iMarblePos);
				q = (dot(q, dir.xyz) * 2.0) * q - dir.xyz;
				vec4 p_temp = vec4(p2+ n*fov*dir.w*2.5, 0);
				vec4 r_temp = vec4(q, dir.w);
				
				refr = render_ray_orig(p_temp, r_temp, fov*1.5);

				//Calculate reflection
				n = -normalize(iMarblePos - cpos.xyz);
				q = dir.xyz - n*(2*dot(dir.xyz,n));
				p_temp = vec4(pos.xyz + n*fov*dir.w*2., 0);
				r_temp = vec4(q, dir.w);
				
				refl = render_ray_orig(p_temp, r_temp, fov*2.);
			}
			
			return lighting_original(color, pbr, vec4(cpos, pos.w), dir, norm, refl, refr, shadow); 
		}
	}
	else
	{
		return sky_color_orig(dir.xyz);
	}
	
}