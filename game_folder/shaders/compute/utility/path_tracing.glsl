
#define MAX_BOUNCE 3.
#define reflection 0.1

vec3 path_march(vec4 p, vec4 ray, vec4 var, float angle, float seed)
{
    vec3 fincol = vec3(1.), finill = vec3(0.);
    vec4 res = vec4(0.);
    for(float b = 0.; (b < MAX_BOUNCE); b++)
    {
        if(b < 1.)
        {
            float h = DE(p.xyz);
            if (h < angle*ray.w || ray.w > MAX_DIST)
            {
                 res = vec4(p.xyz, h);
            }
        }
       
        if(res.xyz != p.xyz)
        {
            //march next ray
       		ray_march(p, ray, var, angle);
			res = p;
        }
         
        if(ray.w > MAX_DIST || (var.x >= 200 && res.w > 5.*angle*ray.w))
        {
            finill += sky_color(ray.xyz)*fincol;
            break;
        }
        
        /// Surface interaction
        vec3 norm = normalize(calcNormalA(res.xyz, res.w));    
        //discontinuity correction
        p.xyz = res.xyz - (res.w - 1.2*angle*ray.w)*norm;
        
        vec3 refl = reflect(ray.xyz, norm);
        
        float refl_prob = hash(seed*SQRT2);
       
        //random diffusion, random distr already samples cos(theta) closely
        if(refl_prob < reflection)
        {
            vec3 rand = clamp(pow(1.-reflection,4.)*randn(seed*SQRT3),-1.,1.);
        	ray.xyz = normalize(refl + rand);
        }
        else
        {
            ray.xyz = cosdistr(norm, seed*PI);
        }
      

        //color and illuminaition
        vec4 colp = COL(p.xyz);
        p.xyz = p.xyz - (colp.w - 1.2*angle*ray.w)*norm;
        fincol = fincol*clamp(colp.xyz,0.,1.);
        
		
        //OOF levels of inefficiency 
		vec3 sunc = sky_color(normalize(LIGHT_DIRECTION));
		if(b > 0.)
        finill += sunc*shadow_march(vec4(p.xyz, MIN_DIST), vec4(normalize(LIGHT_DIRECTION),0), 10., 0.3)*fincol;
		
		//add fractal glow
       // finill += 500000.*exp(-300.*clamp(pow(abs(length(colp.xyz-vec3(0.5,0.5,0.8))),2.),0.,1.))*fincol;
        
       // angle *= 1.15;
    }
    
    return finill;
}
