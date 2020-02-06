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

ray get_ray(glcamera cam, vec2 screen_pos)
{
	float delta = 0;
	if(cam.cross_eye)
	{
		delta = cam.eye_separation*(2.f*floor(2.f*screen_pos.x)-1.f);
		screen_pos.x = 0.5*(mod(2*screen_pos.x,1.f)+0.5);
	}	
	
	vec2 shift = cam.FOV*(2.f*screen_pos - 1.f)*vec2(cam.aspect_ratio, -1.f);
	ray cray;
	cray.pos = cam.position + (cam.cross_eye?(cam.dirx*delta):(cam.size*(cam.dirx*(shift.x) + cam.diry*shift.y)));
	cray.dir = normalize(cam.dirx*shift.x + cam.diry*shift.y + cam.dirz);
	float aspect_ratio_ratio = cam.aspect_ratio/(cam.resolution.x/cam.resolution.y);
	fovray = 1.41*cam.FOV*max(1.f/aspect_ratio_ratio, aspect_ratio_ratio)/cam.resolution.x; //pixel FOV
	return cray;
}

float getTD(vec3 pos, vec2 screen_pos)
{
	float delta = 0;
	if(Camera.cross_eye)
	{
		delta = Camera.eye_separation*(2.f*floor(2.f*screen_pos.x)-1.f);
		screen_pos.x = 0.5*(mod(2*screen_pos.x,1.f)+0.5);
	}	
	
	vec2 shift = Camera.FOV*(2.f*screen_pos - 1.f)*vec2(Camera.aspect_ratio, -1.f);
	return length(Camera.position + (Camera.cross_eye?(Camera.dirx*delta):(Camera.size*(Camera.dirx*(shift.x) + Camera.diry*shift.y))) - pos);
}

ray get_ray(vec2 screen_pos)
{
	return get_ray(Camera, screen_pos);
}

//reprojection cost function, distance 
float rd(glcamera cam, vec3 point, vec2 UV)
{
	ray guess = get_ray(cam, UV);
	vec3 a = normalize(point - guess.pos);
	return dot(guess.dir - a, guess.dir - a);
}

#define duv 0.0001
vec2 reproject_gradient(glcamera cam, vec3 point, vec2 UV)
{
	return vec2(rd(cam, point, UV + vec2(duv,0)) - rd(cam, point, UV - vec2(duv,0)), rd(cam, point, UV + vec2(0,duv)) - rd(cam, point, UV - vec2(0,duv)))/(2*duv);
}

//gradient descent step
#define gdstep 0.06


vec2 reproject_step_gd(glcamera cam, vec3 point, vec2 UV)
{
	//gradient descent, works for any camera
	UV -= gdstep*reproject_gradient(cam, point, UV);
	return UV;
}


vec2 reproject_step(glcamera cam, vec3 point, vec2 UV)
{
	//exact reprojection, limited to a simple camera
	ray guess = get_ray(cam, UV);
	vec3 a = normalize(point - guess.pos);
	if(cam.cross_eye)
	{
		float dUV = 0.;
		dUV = floor(2.f*UV.x);
		UV = vec2(dot(a,normalize(cam.dirx))/cam.aspect_ratio,-dot(a,normalize(cam.diry)))/(2.*cam.FOV*dot(a,normalize(cam.dirz))) + vec2(0.25+0.5*dUV,0.5);
	}
	else
	{
		UV = vec2(dot(a,normalize(cam.dirx))/cam.aspect_ratio,-dot(a,normalize(cam.diry)))/(2.*cam.FOV*dot(a,normalize(cam.dirz))) + 0.5;
	}
	return UV;
}
	
	

//find the previous uv coordinate
vec2 reproject(vec3 point, vec2 UV)
{
	for(int i = 0; i < 5; i++)
	{
		UV = reproject_step(PrevCamera, point, UV);
	}
	for(int i = 0; i < 4; i++)
	{
		UV = reproject_step_gd(PrevCamera, point, UV);
	}
	return UV*PrevCamera.resolution.xy;
}

//find the uv coordinate for a point in space
vec2 project(vec3 point, vec2 UV)
{
	for(int i = 0; i < 5; i++)
	{
		UV = reproject_step(Camera, point, UV);
	}
	for(int i = 0; i < 4; i++)
	{
		UV = reproject_step_gd(Camera, point, UV);
	}
	return UV*Camera.resolution.xy;
}

ray get_ray(inout vec2 screen_pos, float noise)
{
	screen_pos += noise*(hash22(screen_pos + float(iFrame%10000))-0.5)/Camera.resolution;
	return get_ray(screen_pos);
}