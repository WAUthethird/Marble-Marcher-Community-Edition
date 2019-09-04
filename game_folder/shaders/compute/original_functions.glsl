#define DIFFUSE_ENABLED 0
#define DIFFUSE_ENHANCED_ENABLED 1
#define FILTERING_ENABLE 0
#define FOCAL_DIST 1.73205080757
#define FOG_ENABLED 0
#define MAX_DIST 30.0
#define MAX_MARCHES 512
#define MIN_DIST 1e-5
#define PI 3.14159265358979
#define SHADOW_DARKNESS 0.7
#define SHADOW_SHARPNESS 10.0
#define SPECULAR_HIGHLIGHT 40
#define SPECULAR_MULT 0.25
#define SUN_ENABLED 1
#define SUN_SHARPNESS 2.0
#define SUN_SIZE 0.004
#define VIGNETTE_STRENGTH 0.5
#define AMBIENT_OCCLUSION_COLOR_DELTA vec3(0.5)
#define AMBIENT_OCCLUSION_STRENGTH 0.008
#define BACKGROUND_COLOR vec3(0.6,0.8,1.0)

#define COL col_scene
#define DE de_scene

uniform float iFracScale;
uniform float iFracAng1;
uniform float iFracAng2;
uniform vec3 iFracShift;
uniform vec3 iFracCol;
uniform vec3 iMarblePos;
uniform float iMarbleRad;
uniform float iFlagScale;
uniform vec3 iFlagPos;
uniform float iExposure;

uniform vec3 LIGHT_DIRECTION;
uniform bool SHADOWS_ENABLED; 
uniform float CAMERA_SIZE;
uniform int FRACTAL_ITER;
uniform bool REFL_REFR_ENABLED;
uniform int MARBLE_MODE;
#define LIGHT_COLOR vec3(1.0,0.95,0.8)

float FOVperPixel;

float s1 = sin(iFracAng1), c1 = cos(iFracAng1), s2 = sin(iFracAng2), c2 = cos(iFracAng2);


vec3 refraction(vec3 rd, vec3 n, float p) {
	float dot_nd = dot(rd, n);
	return p * (rd - dot_nd * n) + sqrt(1.0 - (p * p) * (1.0 - dot_nd * dot_nd)) * n;
}

//##########################################
//   Space folding
//##########################################
void planeFold(inout vec4 z, vec3 n, float d) {
	z.xyz -= 2.0 * min(0.0, dot(z.xyz, n) - d) * n;
}
void sierpinskiFold(inout vec4 z) {
	z.xy -= min(z.x + z.y, 0.0);
	z.xz -= min(z.x + z.z, 0.0);
	z.yz -= min(z.y + z.z, 0.0);
}
void mengerFold(inout vec4 z) {
	float a = min(z.x - z.y, 0.0);
	z.x -= a;
	z.y += a;
	a = min(z.x - z.z, 0.0);
	z.x -= a;
	z.z += a;
	a = min(z.y - z.z, 0.0);
	z.y -= a;
	z.z += a;
}
void boxFold(inout vec4 z, vec3 r) {
	z.xyz = clamp(z.xyz, -r, r) * 2.0 - z.xyz;
}
void rotX(inout vec4 z, float s, float c) {
	z.yz = vec2(c*z.y + s*z.z, c*z.z - s*z.y);
}
void rotY(inout vec4 z, float s, float c) {
	z.xz = vec2(c*z.x - s*z.z, c*z.z + s*z.x);
}
void rotZ(inout vec4 z, float s, float c) {
	z.xy = vec2(c*z.x + s*z.y, c*z.y - s*z.x);
}
void rotX(inout vec4 z, float a) {
	rotX(z, sin(a), cos(a));
}
void rotY(inout vec4 z, float a) {
	rotY(z, sin(a), cos(a));
}
void rotZ(inout vec4 z, float a) {
	rotZ(z, sin(a), cos(a));
}

//##########################################
//   Primitive DEs
//##########################################
float de_sphere(vec4 p, float r) {
	return (length(p.xyz) - r) / p.w;
}
float de_box(vec4 p, vec3 s) {
	vec3 a = abs(p.xyz) - s;
	return (min(max(max(a.x, a.y), a.z), 0.0) + length(max(a, 0.0))) / p.w;
}
float de_tetrahedron(vec4 p, float r) {
	float md = max(max(-p.x - p.y - p.z, p.x + p.y - p.z),
				max(-p.x + p.y + p.z, p.x - p.y + p.z));
	return (md - r) / (p.w * sqrt(3.0));
}
float de_capsule(vec4 p, float h, float r) {
	p.y -= clamp(p.y, -h, h);
	return (length(p.xyz) - r) / p.w;
}

//##########################################
//   Main DEs
//##########################################
float de_fractal(vec4 p) {
	for (int i = 0; i < FRACTAL_ITER; ++i) {
		p.xyz = abs(p.xyz);
		rotZ(p, s1, c1);
		mengerFold(p);
		rotX(p, s2, c2);
		p *= iFracScale;
		p.xyz += iFracShift;
	}
	return de_box(p, vec3(6.0));
}

vec4 col_fractal(vec4 p) {
	vec3 orbit = vec3(0.0);
	for (int i = 0; i < FRACTAL_ITER; ++i) {
		p.xyz = abs(p.xyz);
		rotZ(p, s1, c1);
		mengerFold(p);
		rotX(p, s2, c2);
		p *= iFracScale;
		p.xyz += iFracShift;
		orbit = max(orbit, p.xyz*iFracCol);
	}
	return vec4(orbit, de_box(p, vec3(6.0)));
}
float de_marble(vec4 p) {
	return de_sphere(p - vec4(iMarblePos, 0), iMarbleRad);
}
vec4 col_marble(vec4 p) {
	return vec4(0, 0, 0, de_sphere(p - vec4(iMarblePos, 0), iMarbleRad));
}

float de_flag(vec4 p) {
	vec3 f_pos = iFlagPos + vec3(1.5, 4, 0)*iFlagScale;
	float d = de_box(p - vec4(f_pos, 0), vec3(1.5, 0.8, 0.08)*iMarbleRad);
	d = min(d, de_capsule(p - vec4(iFlagPos + vec3(0, iFlagScale*2.4, 0), 0), iMarbleRad*2.4, iMarbleRad*0.18));
	return d;
}
vec4 col_flag(vec4 p) {
	vec3 f_pos = iFlagPos + vec3(1.5, 4, 0)*iFlagScale;
	float d1 = de_box(p - vec4(f_pos, 0), vec3(1.5, 0.8, 0.08)*iMarbleRad);
	float d2 = de_capsule(p - vec4(iFlagPos + vec3(0, iFlagScale*2.4, 0), 0), iMarbleRad*2.4, iMarbleRad*0.18);
	if (d1 < d2) {
		return vec4(1.0, 0.2, 0.1, d1);
	} else {
		return vec4(0.9, 0.9, 0.1, d2);
	}
}
float de_scene(vec4 p) {
	float d = de_fractal(p);
	d = min(d, de_marble(p));
	d = min(d, de_flag(p));
	return d;
}
vec4 col_scene(vec4 p) {
	vec4 col = col_fractal(p);
	vec4 col_f = col_flag(p);
	if (col_f.w < col.w) { col = col_f; }
	vec4 col_m = col_marble(p);
	if (col_m.w < col.w) {
		return vec4(col_m.xyz, 1.0);
	}
	return vec4(col.xyz, 0.0);
}

//##########################################
//   Main code
//##########################################

//A faster formula to find the gradient/normal direction of the DE
//credit to http://www.iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec3 calcNormal(vec4 p, float dx) {
	const vec3 k = vec3(1,-1,0);
	return normalize(k.xyy*DE(p + k.xyyz*dx) +
					 k.yyx*DE(p + k.yyxz*dx) +
					 k.yxy*DE(p + k.yxyz*dx) +
					 k.xxx*DE(p + k.xxxz*dx));
}


vec4 ray_march(inout vec4 p, vec4 dir, float sharpness) {
	//March the ray
	float d = DE(p);
	float s = 0.0;
	float td = 0.0;
	float min_d = 1.0;
	for (; s < MAX_MARCHES; s += 1.0) {
		//if the distance from the surface is less than the distance per pixel we stop
		float min_dist = max(FOVperPixel*td, MIN_DIST);
		if(d < -min_dist || td > MAX_DIST)
		{
			break;
		}
		else if (d < min_dist)
		{
			s += d / min_dist;
			break;
		} 
		td += d+min_dist*0.1;
		p += dir * (d+min_dist*0.1);
		min_d = min(min_d, sharpness * d / td);
		d = DE(p);
	}
	return vec4(d, s, td, min_d);
}


vec4 scene(inout vec4 p, inout vec4 dir, float vignette) {
	//Trace the dir
	vec4 d_s_td_m = ray_march(p, dir, 1.0f);
	float d = d_s_td_m.x;
	float s = d_s_td_m.y;
	float td = d_s_td_m.z;

	//Determine the color for this pixel
	vec4 col = vec4(0.0);
	float min_dist = max(FOVperPixel*td, MIN_DIST);
	if (d < min_dist) {
		//Get the surface normal
		vec3 n = calcNormal(p, min_dist*0.5);
		
		//find closest surface point, without this we get weird coloring artifacts
		if(s>0) p.xyz -= n*d;

		//Get coloring
		#if FILTERING_ENABLE
			//sample direction 1, the cross product between the dir and the surface normal, should be parallel to the surface
			vec3 s1 = normalize(cross(dir.xyz, n));
			//sample direction 2, the cross product between s1 and the surface normal
			vec3 s2 = cross(s1, n);
			//get filtered color
			vec4 orig_col = clamp(smoothColor(p, s1, s2, min_dist*0.5), 0.0, 1.0);
		#else
			vec4 orig_col = clamp(COL(p), 0.0, 1.0);
		#endif
		col.w = orig_col.w;

	
		//Get if this point is in shadow
		float k = 1.0;
		if(SHADOWS_ENABLED)
		{
			vec4 light_pt = p;
			light_pt.xyz += n * MIN_DIST * 100;
			vec4 rm = ray_march(light_pt, vec4(LIGHT_DIRECTION, 0.0), SHADOW_SHARPNESS);
			k = rm.w * min(rm.z, 1.0);
		}

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
		col.xyz += orig_col.xyz * LIGHT_COLOR * k;

		//Add small amount of ambient occlusion
		float a = 1.0 / (1.0 + s * AMBIENT_OCCLUSION_STRENGTH);
		col.xyz += (1.0 - a) * AMBIENT_OCCLUSION_COLOR_DELTA;

		//Add fog effects
		#if FOG_ENABLED
			float b = td / MAX_DIST;
			col.xyz = (1.0 - b) * col.xyz + b * BACKGROUND_COLOR;
		#endif
			
		//Return normal through ray
		dir = vec4(n, 0.0);
	} else {
		//Ray missed, start with solid background color
		col.xyz += BACKGROUND_COLOR;

		col.xyz *= vignette;
		//Background specular
		#if SUN_ENABLED
			float sun_spec = dot(dir.xyz, LIGHT_DIRECTION) - 1.0 + SUN_SIZE;
			sun_spec = min(exp(sun_spec * SUN_SHARPNESS / SUN_SIZE), 1.0);
			col.xyz += LIGHT_COLOR * sun_spec;
		#endif
	}

	return col;
}