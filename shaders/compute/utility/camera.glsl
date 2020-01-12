
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