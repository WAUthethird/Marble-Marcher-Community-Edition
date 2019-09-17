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
uniform int FRACTAL_ITER;
uniform int MARBLE_MODE;
uniform float time;

uniform float PBR_METALLIC;
uniform float PBR_ROUGHNESS;

layout(rgba8, binding = 4) uniform image2D color_flag; 

///Original MM distance estimators

float s1 = sin(iFracAng1), c1 = cos(iFracAng1), s2 = sin(iFracAng2), c2 = cos(iFracAng2);

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

// Polynomial smooth minimum by iq
float smoothmin(float a, float b, float k) {
  float h = clamp(0.5 + 0.5*(a-b)/k, 0.0, 1.0);
  return mix(a, b, h) - k*h*(1.0-h);
}

/*void mengerFold(inout vec4 z) {
	float a = smoothmin(z.x - z.y, 0.0, 0.03);
	z.x -= a;
	z.y += a;
	a = smoothmin(z.x - z.z, 0.0, 0.03);
	z.x -= a;
	z.z += a;
	a = smoothmin(z.y - z.z, 0.0, 0.03);
	z.y -= a;
	z.z += a;
}*/


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
float de_fractal(vec4 p)
{
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

vec4 col_fractal(vec4 p) 
{
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

float de_marble(vec4 p) 
{
	return de_sphere(p - vec4(iMarblePos, 0), iMarbleRad);
}

vec4 col_marble(vec4 p) 
{
	vec4 col = vec4(0.5, 0.5, 0.5, de_sphere(p - vec4(iMarblePos, 0), iMarbleRad));
	return col;
}

float de_flag(vec4 p) 
{
	vec3 f_pos = iFlagPos + vec3(1.5, 4, 0)*iFlagScale;
	vec4 p_s = p/iMarbleRad;
	vec4 d_pos = p - vec4(f_pos, 0);
	vec4 caps_pos = p - vec4(iFlagPos + vec3(0, iFlagScale*2.4, 0), 0);
	//animated flag woooo
	float speed = 14;
	float oscillation = sin(8*p_s.x - 1*p_s.y - speed*time) + 0.4*sin(11*p_s.x + 2*p_s.y - 1.2*speed*time) + 0.15*sin(20*p_s.x - 5*p_s.y -1.4*speed*time);
	//scale the flag displacement amplitude by the distance from the flagpole
	float d = 0.4*de_box(d_pos + caps_pos.x*vec4(0,(0.02+ caps_pos.x* 0.5+0.01*oscillation),0.04*oscillation,0), vec3(1.5, 0.8, 0.005)*iMarbleRad);
	d = min(d, de_capsule(caps_pos, iMarbleRad*2.4, iMarbleRad*0.05));
	return d;
}

vec4 col_flag(vec4 p) 
{
	vec3 f_pos = iFlagPos + vec3(1.5, 4, 0)*iFlagScale;
	vec4 d_pos = p - vec4(f_pos, 0);
	vec3 fsize = vec3(1.5, 0.8, 0.08)*iMarbleRad;
	vec4 caps_pos = p - vec4(iFlagPos + vec3(0, iFlagScale*2.4, 0), 0);
	float d1 = de_box(d_pos, fsize);
	float d2 = de_capsule(p - vec4(iFlagPos + vec3(0, iFlagScale*2.4, 0), 0), iMarbleRad*2.4, iMarbleRad*0.18);
	if (d1 < d2) {
		vec2 color_flag_s = vec2(imageSize(color_flag));
		vec2 texture_coord = d_pos.xy*vec2(0.5,-0.48)/fsize.xy + vec2(0.5,0.5) - 0.5*vec2(0,caps_pos.x*(0.02+ caps_pos.x* 0.5))/fsize.xy;
		vec3 flagcolor = imageLoad(color_flag, ivec2(texture_coord*color_flag_s)).xyz;
		return vec4(flagcolor, d1);
	} else {
		return vec4(0.9, 0.9, 0.1, d2);
	}
}

//float DE_count = 0;

float de_scene(vec3 pos) 
{
	//DE_count = DE_count+1;
	vec4 p = vec4(pos,1.f);
	float d = de_fractal(p);
	d = min(d, de_marble(p));
	d = min(d, de_flag(p));
	return d;
}

vec4 col_scene(vec3 pos) 
{
	//DE_count = DE_count+1;
	vec4 p = vec4(pos,1.f);
	vec4 col = col_fractal(p);
	vec4 col_f = col_flag(p);
	if (col_f.w < col.w) { col = col_f; }
	vec4 col_m = col_marble(p);
	if (col_m.w < col.w) {
		return vec4(col_m.xyz, 1.0);
	}
	return vec4(min(col.xyz,1), 0.0);
}

void scene_material(vec3 pos, inout vec4 color, inout vec2 pbr)
{
	//DE_count = DE_count+1;
	vec4 p = vec4(pos,1.f);
	
	color = col_fractal(p);
	vec4 color_f = col_flag(p);
	vec4 color_m = col_marble(p);
	
	pbr = vec2(PBR_METALLIC, PBR_ROUGHNESS);
	float reflection = 0;
	
	if (color_f.w < color.w) 
	{ 
		color = color_f; 
		pbr = vec2(0.2,0.35);
	}
	
	if (color_m.w < color.w) 
	{ 
		color = color_m; 
		if(MARBLE_MODE <= 1)
		{
			pbr = vec2(1,0.2);
		}
		else
		{
			pbr = vec2(0,0.2);
		}
		
		reflection = 1;
	}

	color = vec4(min(color.xyz,1), reflection);
}

//A faster formula to find the gradient/normal direction of the DE
//credit to http://www.iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec4 calcNormal(vec3 p, float dx) {
	const vec3 k = vec3(1,-1,0);
	return   (k.xyyx*DE(p + k.xyy*dx) +
			 k.yyxx*DE(p + k.yyx*dx) +
			 k.yxyx*DE(p + k.yxy*dx) +
			 k.xxxx*DE(p + k.xxx*dx))/vec4(4*dx,4*dx,4*dx,4);
}