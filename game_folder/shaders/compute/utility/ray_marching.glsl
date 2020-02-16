#include<utility/distance_estimators.glsl>

/* OLD CODE
void ray_march(inout vec4 pos, inout vec4 dir, inout vec4 var, float fov) 
{
	float pDE = 0;
	//March the ray
	for (; var.x < MAX_MARCHES; var.x += 1.0) {	
		dir.w += pos.w;
		pos.xyz += pos.w*dir.xyz;
		pDE =pos.w;
		pos.w = DE(pos.xyz);
		
		//if the distance from the surface is less than the distance per pixel we stop
		if(dir.w > MAX_DIST || (pos.w < max(fov*dir.w, MIN_DIST) && pDE > pos.w))
		{
			break;
		}
	}
}*/

void ray_march(inout vec4 p, inout vec4 ray, inout vec4 var, float angle, float max_d)
{
    float prev_h = 0., td = 0.;
	vec3 pp = p.xyz;
    float omega = overrelax;
    float candidate_td = 1.;
    float candidate_error = 1e8;
    for(; ((ray.w+td) < max_d) && (var.x < MAX_MARCHES);   var.x+= 1.)
    {
        p.w = DE(p.xyz + td*ray.xyz);
        
        if(prev_h*omega>max(p.w,0.)+max(prev_h,0.)) //if overtepped
        {
            td += (1.-omega)*prev_h; // step back to the safe distance
            prev_h = 0.;
            omega = (omega - 1.)*0.55 + 1.; //make the overstepping smaller
        }
        else
        {	
            if(p.w/td < candidate_error)
            {
				if(p.w < 0.)
				{
					candidate_error = 0.;
					candidate_td = td;
					break;
				}
			
                candidate_error = p.w/td;
                candidate_td = td; 
				
                if(p.w < (ray.w+td)*angle) //if closer to the surface than the cone radius
                {
                    break;
                }
            }
            
            td += p.w*omega; //continue marching
            
            prev_h = p.w;        
        }
    }
    if((ray.w+td) >= max_d) candidate_td = max_d - ray.w;
	
    ray.w += candidate_td;
	p.xyz = p.xyz + candidate_td*ray.xyz;
	p.w = candidate_error*candidate_td;
}


void ray_march(inout vec4 p, inout vec4 ray, inout vec4 var, float angle)
{
	ray_march(p, ray, var, angle, MAX_DIST);
}


void ray_march_limited(inout vec4 pos, inout vec4 dir, inout vec4 var, float d0) 
{
	ray_march(pos, dir, var, d0);
	if((pos.w > 0.) && (dir.w < MAX_DIST) && (var.x < MAX_MARCHES))
	{
		pos.w = DE(pos.xyz) - d0*dir.w;
		for (int i = 0; i < 1; i++)
		{
			pos.xyz += pos.w*dir.xyz;
			dir.w += pos.w;
			pos.w = DE(pos.xyz) - d0*dir.w;
		}
		pos.w += d0*dir.w;
	}
	
}

void ray_march_continue(inout vec4 pos, inout vec4 dir, inout vec4 var, float fov) 
{
	dir.w += pos.w;
	pos.xyz += pos.w*dir.xyz;
	
	if((dir.w > MAX_DIST) || (pos.w < 0.) || (var.x >= MAX_MARCHES))
	{
		return;
	}
	
	ray_march(pos, dir, var, fov);
}
#define shadow_steps 128
float shadow_march(vec4 pos, vec4 dir, float distance2light, float light_angle)
{
	if(!SHADOWS_ENABLED) return 0.;
	
	float light_visibility = 1;
	float ph = 1e5;
	float dDEdt = 0;
	pos.w = DE(pos.xyz);
	int i = 0;
	for (; i < shadow_steps; i++) {
	
		dir.w += pos.w;
		pos.xyz += pos.w*dir.xyz;
		pos.w = DE(pos.xyz);
		
		float y = pos.w*pos.w/(2.0*ph);
        float d = (pos.w+ph)*0.5*(1-dDEdt);
		float angle = d/(max(MIN_DIST,dir.w-y)*light_angle);
		
        light_visibility = min(light_visibility, angle);
		
		//minimizing banding even further
		dDEdt = dDEdt*0.75 + 0.25*(pos.w-ph)/ph;
		
		ph = pos.w;
		
		if(dir.w >= distance2light)
		{
			break;
		}
		
		if(dir.w > MAX_DIST || pos.w < max(fovray*dir.w, MIN_DIST))
		{
			return 0;
		}
	}
	
	if(i >= shadow_steps)
	{
		light_visibility=0.;
	}
	//return light_visibility; //bad
	light_visibility = light_visibility*2. - 1.;
	return 0.5 + (light_visibility*sqrt(1.-light_visibility*light_visibility) + asin(light_visibility))/3.14159265; //looks better and is more physically accurate(for a circular light source)
}

float sphere_intersection(vec3 r, vec3 p, vec4 sphere)
{
	p = p - sphere.xyz;
	if(p == vec3(0)) return sphere.w;
	
	float b = dot(p, r);
	float c = sphere.w*sphere.w - dot(p,p);
	float d = b*b + c;
	
	if((d <= 0) || (c <= 0)) //if no intersection
	{
		return 0;
	}
	else
	{
		return sqrt(d) - b; //use furthest solution in the direction of the ray
	}
}

void normarch(inout vec4 pos)
{
	//calculate the normal
	vec4 pos0 = pos;
	vec4 norm = calcNormal(pos.xyz, pos.w/8); 
	norm.xyz = normalize(norm.xyz);
	pos.w = norm.w;
	
	//march in the direction of the normal
	for(int i = 0; i < NORMARCHES; i++)
	{
		pos.xyz += pos.w*norm.xyz;
		pos.w = DE(pos.xyz);
		//if the normal DE sphere is further than the initial DE sphere
		if(length(pos0.xyz - (pos.xyz+pos.w*norm.xyz))>pos.w+pos0.w)
		{
			break;
		}
	}
}
