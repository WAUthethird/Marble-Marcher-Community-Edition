#include<utility/RNG.glsl>

struct ray
{
	vec3 pos;
	vec3 dir;
};		

struct glcamera
{
	vec3 position;
	vec3 dirx;
	vec3 diry;
	vec3 dirz;
	vec2 resolution;
	float aspect_ratio;
	float FOV;
	float focus;
	float bokeh;
	float exposure;
	float mblur;
	float speckle;
	float size;
	float bloomintensity;
	float bloomradius;
	bool cross_eye;
	float eye_separation;
};

ivec2 getGpos(int index)
{
	int y = index/group_size;
	int x = index%group_size;
	return ivec2(x,y);
}

uniform glcamera Camera;
uniform glcamera PrevCamera;
float fovray;



ray get_ray(vec2 screen_pos)
{
	float delta = 0;
	if(Camera.cross_eye)
	{
		delta = Camera.eye_separation*(2.f*floor(2.f*screen_pos.x)-1.f);
		screen_pos.x = 0.5*(mod(2*screen_pos.x,1.f)+0.5);
	}	
	
	vec2 shift = Camera.FOV*(2.f*screen_pos - 1.f)*vec2(Camera.aspect_ratio, -1.f);
	ray cray;
	cray.pos = Camera.position + (Camera.cross_eye?(Camera.dirx*delta):(Camera.size*(Camera.dirx*(shift.x) + Camera.diry*shift.y)));
	cray.dir = normalize(Camera.dirx*shift.x + Camera.diry*shift.y + Camera.dirz);
	float aspect_ratio_ratio = Camera.aspect_ratio/(Camera.resolution.x/Camera.resolution.y);
	fovray = 1.41*Camera.FOV*max(1.f/aspect_ratio_ratio, aspect_ratio_ratio)/Camera.resolution.x; //pixel FOV
	return cray;
}

ray get_prevray(vec2 screen_pos)
{
	float delta = 0;
	if(PrevCamera.cross_eye)
	{
		delta = PrevCamera.eye_separation*(2.f*floor(2.f*screen_pos.x)-1.f);
		screen_pos.x = 0.5*(mod(2*screen_pos.x,1.f)+0.5);
	}	
	
	vec2 shift = PrevCamera.FOV*(2.f*screen_pos - 1.f)*vec2(PrevCamera.aspect_ratio, -1.f);
	ray cray;
	cray.pos = PrevCamera.position + (PrevCamera.cross_eye?(PrevCamera.dirx*delta):(PrevCamera.size*(PrevCamera.dirx*(shift.x) + PrevCamera.diry*shift.y)));
	cray.dir = normalize(PrevCamera.dirx*shift.x + PrevCamera.diry*shift.y + PrevCamera.dirz);
	float aspect_ratio_ratio = PrevCamera.aspect_ratio/(PrevCamera.resolution.x/PrevCamera.resolution.y);
	fovray = 1.41*PrevCamera.FOV*max(1.f/aspect_ratio_ratio, aspect_ratio_ratio)/PrevCamera.resolution.x; //pixel FOV
	return cray;
}

vec2 reproject_step(vec3 point, vec2 UV)
{
	ray guess = get_prevray(UV);
	vec3 a = normalize(point - guess.pos);
	UV = vec2(dot(a,normalize(PrevCamera.dirx))/Camera.aspect_ratio,-dot(a,normalize(PrevCamera.diry)))/(2.*PrevCamera.FOV*dot(a,normalize(PrevCamera.dirz))) + 0.5;
	if(PrevCamera.cross_eye)
	{
		UV.x *= 0.5;
	}
	return UV;
}

//find the previous uv coordinate
vec2 reproject(vec3 point, vec2 UV)
{
	for(int i = 0; i < 5; i++)
	{
		UV = reproject_step(point, UV);
	}
	return UV*PrevCamera.resolution.xy;
}

ray get_ray(vec2 screen_pos, float noise)
{
	return get_ray(screen_pos + noise*(hash22(screen_pos + float(iFrame%10000))-0.5)/Camera.resolution.x);
}