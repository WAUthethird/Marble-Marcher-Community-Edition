vec3 path_march(vec4 p, inout vec4 dir, vec4 var, float angle, float seed, int skipsun, int skipbounce)
{
    vec3 fincol = vec3(1.), finill = vec3(0.);
    vec3 fdir = vec3(1,0.,0.);
    for(float b = 0.; (b < MAX_BOUNCE); b++)
    {
		seed += 5.35;
        if(b >= 1.)
		{
			//march next dir
       		ray_march(p, dir, var, angle);
		}
         
		if(p.w < 0)
		{
			break;
		}
		 
        if(dir.w > 0.5*MAX_DIST || (var.x >= 200 && p.w > 5.*angle*dir.w))
        {
            finill += sky_color(dir.xyz)*fincol;
			
            break;
        }
		
        /// Surface interaction
        vec3 norm = normalize(calcNormal(p.xyz, 4.*angle*dir.w).xyz);    
        //discontinuity correction
        p.xyz = p.xyz + (p.w - 1.*angle*dir.w)*dir.xyz;
        
        vec3 refl = reflect(dir.xyz, norm);
        
        float refl_prob = hash(seed*SQRT2);
		float sun_prob = pow(hash(seed*SQRT5),4.);
		
		vec3 incoming = dir.xyz;
		
	   	vec4 colp; vec2 pbr; vec3 emission;
		scene_material(p.xyz - DE(p.xyz)*norm, colp, pbr, emission);
		
        p.xyz = p.xyz + (colp.w - 1.2*angle*dir.w)*incoming;
		
		float roughness = pbr.y;
		float reflection = max(0.05, 1.-roughness);
		
        //random diffusion, random distr already samples cos(theta) closely
		if(refl_prob < reflection)
        {
            vec3 rand = clamp(pow(1.-reflection,3.)*randn(seed*SQRT3),-1.,1.);
        	dir.xyz = normalize(refl + rand);
        }
        else
        {
            dir.xyz = cosdistr(norm, seed*PI);
        }
		
		if(b < 1.)	fdir = dir.xyz;
		
		vec3 albedo = colp.xyz;
		albedo = pow(abs(albedo),vec3(1.f/gamma_material)); //square it to make the fractals more colorfull 
		
		vec3 V = -incoming;
		
		if(dot(LIGHT_DIRECTION, norm.xyz) > 0 && b >= skipsun)
        {
			vec3 L = normalize(LIGHT_DIRECTION);
			vec3 H = normalize(V + L);
			
			// add to the color            
			float NdotL = max(dot(norm.xyz, L), 0.0);    
			vec3 ill = sky_color(L)*shadow_march(p, vec4(L,0), MAX_DIST, LIGHT_ANGLE);
			finill += clamp(BRDF(V, L, norm.xyz, albedo, pbr),0.,5.)*ill*fincol*NdotL;
		}
		
		//random reflected ray
		if(b >= skipbounce)
		{
			vec3 L = dir.xyz;
			vec3 H = normalize(V + L);
			
			// add to the color       
			finill += emission*fincol;			
			fincol *= clamp(BRDF(V, L, norm.xyz, albedo, pbr),0.,3.);
		}
    }
    dir.xyz = fdir;
    return finill;
}

vec3 path_march(vec4 p, inout vec4 dir, vec4 var, float angle, float seed)
{
	return path_march(p, dir, var, angle, seed, 1, 0);
}

//position, camera direction, previous light direction, average neighbor pixel direction, var, seed
vec3 adaptive_path_march(vec4 p, inout vec4 dir, vec4 pdir, vec4 adir, vec4 var, float angle, float seed)
{
    vec3 fincol = vec3(1.), finill = vec3(0.);
    vec3 fdir = vec3(1,0.,0.);
    for(float b = 0.; (b < MAX_BOUNCE); b++)
    {
		//update seed
		seed += 5.35;
		
		//skip marching first ray, since the point is already at the surface
        if(b >= 1.)
		{
			//march next dir
       		ray_march(p, dir, var, angle);
		}
         
		//if point inside fractal - stop tracing
		if(p.w < 0)
		{
			break;
		}
		 
		//out of bounds - sample sky color
        if(dir.w > 0.5*MAX_DIST || (var.x >= 200 && p.w > 5.*angle*dir.w))
        {
            finill += sky_color(dir.xyz)*fincol;
            break;
        }
		
		//find surface normal
        vec3 norm = normalize(calcNormal(p.xyz, angle*dir.w).xyz); 		
		
        //discontinuity correction
        p.xyz = p.xyz + (p.w - 1.*angle*dir.w)*dir.xyz;
		
        //reflection direction
        vec3 refl = reflect(dir.xyz, norm);
       
        float refl_prob = hash(seed*SQRT2);
		
		vec3 incoming = dir.xyz;
		
		//surface material
	   	vec4 colp; vec2 pbr; vec3 emission;
		scene_material(p.xyz - DE(p.xyz)*norm, colp, pbr, emission);
		
		float roughness = pbr.y;
		float reflection = max(0.05, 1.-roughness);
		
      
		if(b<1.) //dont do a normal distribution for the first adaptive bounce
		{
		
		}
		else
		{
			if(refl_prob < reflection) 
			{
				//random specular reflection, biased
				vec3 rand = clamp(pow(1.-reflection,3.)*randn(seed*SQRT3),-1.,1.);
				dir.xyz = normalize(refl + rand);
			}
			else
			{
				//random diffusion, random distr already samples cos(theta) closely
				dir.xyz = cosdistr(norm, seed*PI);
			}
		}
		
		
		if(b < 1.)	fdir = dir.xyz; // save the 
		
		vec3 albedo = colp.xyz;
		albedo = pow(abs(albedo),vec3(1.f/gamma_material)); //square it to make the fractals more colorfull 
		
		vec3 V = -incoming;
		
		if(dot(LIGHT_DIRECTION, norm.xyz) > 0 && b >= 1)
        {
			vec3 L = normalize(LIGHT_DIRECTION);
			vec3 H = normalize(V + L);
			
			// add to the color            
			float NdotL = max(dot(norm.xyz, L), 0.0);       
			vec3 ill = sky_color(L)*shadow_march(p, vec4(L,0), MAX_DIST, LIGHT_ANGLE);
			finill += clamp(BRDF(V, L, norm.xyz, albedo, pbr),0.,2.5)*ill*fincol*NdotL;
		}
		
		//random reflected ray
		if(b >= 1)
		{
			vec3 L = dir.xyz;
			vec3 H = normalize(V + L);
			
			// add to the color       
			finill += emission*fincol;			
			fincol *= clamp(BRDF(V, L, norm.xyz, albedo, pbr),0.,2.5);
		}
    }
    dir.xyz = fdir;
    return finill;
}