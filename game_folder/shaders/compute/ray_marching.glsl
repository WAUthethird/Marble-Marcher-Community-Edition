#include<distance_estimators.glsl>

#define MAX_DIST 500
#define MIN_DIST 1e-5
#define MAX_MARCHES 700
#define NORMARCHES 8


void ray_march(inout vec4 pos, inout vec4 dir, inout vec4 var, float fov) 
{
	//March the ray
	for (; var.x < MAX_MARCHES; var.x += 1.0) {	
		dir.w += pos.w;
		pos.xyz += pos.w*dir.xyz;
		pos.w = DE(pos.xyz);
		
		//if the distance from the surface is less than the distance per pixel we stop
		if(dir.w > MAX_DIST || pos.w < max(fov*dir.w, MIN_DIST))
		{
			break;
		}
	}
}

void ray_march_limited(inout vec4 pos, inout vec4 dir, inout vec4 var, float fov, float d0) 
{

	pos.w = DE(pos.xyz)-d0*dir.w;
	
	for (; var.x < MAX_MARCHES; var.x += 1.0) {
		//if the distance from the surface is less than the distance per pixel we stop
		if(dir.w > MAX_DIST)
		{
			break;
		}
		
		if(pos.w < max(fov*dir.w, MIN_DIST))
		{
			break;
		}
	
		dir.w += pos.w;
		pos.xyz += pos.w*dir.xyz;
	
		pos.w = DE(pos.xyz)-d0*dir.w;
	}
	
	pos.w += d0*dir.w;
}

void ray_march_continue(inout vec4 pos, inout vec4 dir, inout vec4 var, float fov) 
{
	dir.w += pos.w;
	pos.xyz += pos.w*dir.xyz;
	
	if(dir.w > MAX_DIST)
	{
		return;
	}
	
	pos.w = DE(pos.xyz);
	
	//March the ray
	for (; var.x < MAX_MARCHES; var.x += 1.0) {
		//if the distance from the surface is less than the distance per pixel we stop
		if(dir.w > MAX_DIST || pos.w < max(fov*dir.w, MIN_DIST))
		{
			break;
		}
		
		dir.w += pos.w;
		pos.xyz += pos.w*dir.xyz;
		pos.w = DE(pos.xyz);
	}
}

float shadow_march(vec4 pos, vec4 dir, float distance2light, float light_angle)
{
	float light_visibility = 1;
	float ph = 1e5;
	float dDEdt = 0;
	pos.w = DE(pos.xyz);
	int i = 0;
	for (; i < MAX_MARCHES; i++) {
	
		dir.w += pos.w;
		pos.xyz += pos.w*dir.xyz;
		pos.w = DE(pos.xyz);
		
		float y = pos.w*pos.w/(2.0*ph);
        float d = (pos.w+ph)*0.5*(1-dDEdt);
		float angle = d/(max(MIN_DIST,dir.w-y)*light_angle);
		
        light_visibility = min(light_visibility, angle);
		
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
	//return light_visibility; //bad
	return 0.5-cos(3.14159265*light_visibility)*0.5; //looks better and is more physically accurate(for a circular light source)
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

float find_furthest_intersection(vec3 r, vec3 p, ivec2 id)
{
	float d = 0;
	ivec2 idR = min(id+1, group_size-1);
	ivec2 idL = max(id-1, 0);
	for(int i = idL.x; i <= idR.x; i++)
	{
		for(int j = idL.y; j <= idR.y; j++)
		{
			d = max(d, sphere_intersection(r,p,de_sph[i][j]));
		}
	}
	return d;
}


float find_furthest_intersection_all(vec3 r, vec3 p, ivec2 id)
{
	float d = 0;
	for(int i = 0; i < group_size; i++)
	{
		for(int j = 0; j < group_size; j++)
		{
			d = max(d, sphere_intersection(r,p,de_sph[i][j]));
		}
	}
	return d;
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
