
#define MAX_BOUNCE 4.

vec3 path_march(vec4 p, vec4 dir, vec4 var, float angle, float seed)
{
    vec3 fincol = vec3(1.), finill = vec3(0.);
    vec4 res = vec4(0.);
    for(float b = 0.; (b < MAX_BOUNCE); b++)
    {
        if(b < 1.)
        {
            float h = DE(p.xyz);
            res = vec4(p.xyz, h);
        }
		else
		{
			//march next dir
       		ray_march(p, dir, var, angle);
			res = p;
		}
         
        if(dir.w > 0.1*MAX_DIST || (var.x >= 200 && res.w > 5.*angle*dir.w))
        {
            finill += sky_color(dir.xyz)*fincol;
            break;
        }
        
        /// Surface interaction
        vec3 norm = normalize(calcNormal(res.xyz, angle*dir.w).xyz);    
        //discontinuity correction
        p.xyz = res.xyz - (res.w - 1.*angle*dir.w)*norm;
        
        vec3 refl = reflect(dir.xyz, norm);
        
        float refl_prob = hash(seed*SQRT2);
		
		vec3 incoming = dir.xyz;
		
	   	vec4 colp; vec2 pbr; vec3 emission;
		scene_material(p.xyz - DE(p.xyz)*norm, colp, pbr, emission);
		finill += emission*fincol;
		
        p.xyz = p.xyz + (colp.w - 1.2*angle*dir.w)*incoming;
		
		float roughness = pbr.y;
		float metallic = pbr.x;
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
      
		
		vec3 albedo = colp.xyz;
		if(b < 1.) albedo = vec3(1.); //color demodulation 
		albedo = pow(albedo,vec3(1.f/gamma_material)); //square it to make the fractals more colorfull 
		vec3 F0 = vec3(0.04); 
	    F0 = mix(F0, albedo, metallic);
		
		vec3 V = -incoming;
		
		//sun test ray
		if(b > 0.) //dont count the the first bounce, since that is already rendered
		{
			//sun samples
			vec3 sunc = sky_color(normalize(LIGHT_DIRECTION));
			vec3 sun = sunc*shadow_march(vec4(p.xyz, MIN_DIST), vec4(normalize(LIGHT_DIRECTION),0), 10., 0.3)*fincol;
			
			vec3 L = normalize(LIGHT_DIRECTION);
			vec3 H = normalize(V + L);
			
			//Physically Based Rendering 
			// cook-torrance brdf
			float NDF = DistributionGGX(norm, H, roughness);        
			float G   = GeometrySmith(norm, V, L, roughness);      
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
			
			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - metallic;	  
			
			vec3 numerator    = NDF * G * F;
			float denominator = 4.0 * max(dot(norm, V), 0.0) * max(dot(norm, L), 0.0);
			vec3 specular     = numerator / max(denominator, 0.001);  
				
			// add to the illumination            
			finill += clamp((kD * albedo / PI + specular)*fincol*sun,0.,2.5);
		}
		
		//random reflected ray
		{
			vec3 L = dir.xyz;
			vec3 H = normalize(V + L);
			
			//Physically Based Rendering 
			// cook-torrance brdf
			float NDF = DistributionGGX(norm, H, roughness);        
			float G   = GeometrySmith(norm, V, L, roughness);      
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
			
			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - metallic;	  
			
			vec3 numerator    = NDF * G * F;
			float denominator = 4.0 * max(dot(norm, V), 0.0) * max(dot(norm, L), 0.0);
			vec3 specular     = numerator / max(denominator, 0.001);  
				
			// add to the color            
			fincol *= clamp((kD * albedo / PI + specular),0.,2.5);
		}
    }
    
    return finill;
}
