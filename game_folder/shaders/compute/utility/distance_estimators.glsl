uniform sampler2D iTexture0;
uniform sampler2D iTexture1;

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

vec2 mp = vec2(-1.,1.);
void mengerFold(inout vec4 z) 
{
	z.xy += min(z.x - z.y, 0.0)*mp;
	z.xz += min(z.x - z.z, 0.0)*mp;
	z.yz += min(z.y - z.z, 0.0)*mp;
}

void boxFold(inout vec4 z, vec3 r) {
	z.xyz = clamp(z.xyz, -r, r) * 2.0 - z.xyz;
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
	mat2 rmZ = mat2(c1, s1, -s1, c1);
	mat2 rmX = mat2(c2, s2, -s2, c2);
	for (int i = 0; i < FRACTAL_ITER; ++i) {
		p.xyz = abs(p.xyz);
		p.xy *= rmZ;
		mengerFold(p);
		p.yz *= rmX;
		p *= iFracScale;
		p.xyz += iFracShift;
	}
	return de_box(p, vec3(6.0));
}
/*
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
}*/

vec4 col_fractal(vec4 p) 
{
	vec3 orbit = vec3(0.0);
	mat2 rmZ = mat2(c1, s1, -s1, c1); 
	mat2 rmX = mat2(c2, s2, -s2, c2);
	for (int i = 0; i < FRACTAL_ITER; ++i) {
		p.xyz = abs(p.xyz);
		p.xy *= rmZ; //rotation around z
		mengerFold(p);
		p.yz *= rmX; //rotation around x
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
		vec2 texture_coord = d_pos.xy*vec2(0.5,-0.48)/fsize.xy + vec2(0.5,0.5) - 0.5*vec2(0,caps_pos.x*(0.02+ caps_pos.x* 0.5))/fsize.xy;
		vec3 flagcolor = texture(iTexture0, texture_coord).xyz;
		return vec4(flagcolor, d1);
	} else {
		return vec4(0.9, 0.9, 0.1, d2);
	}
}

float de_scene(vec3 pos) 
{
	vec4 p = vec4(pos,1.f);
	float d = de_fractal(p);
	d = min(d, de_marble(p));
	d = min(d, de_flag(p));
	return d;
}

vec4 col_scene(vec3 pos) 
{
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

void scene_material(vec3 pos, inout vec4 color, inout vec2 pbr, inout vec3 emission)
{
	//DE_count = DE_count+1;
	vec4 p = vec4(pos,1.f);
	emission = vec3(0.);
	
	color = col_fractal(p);
	vec4 color_f = col_flag(p);
	vec4 color_m = col_marble(p);
	
	pbr = vec2(PBR_METALLIC, PBR_ROUGHNESS);
	float reflection = 0;

	if(FRACTAL_GLOW)
	{
		vec3 hfcol = 0.5*sin(vec3(3.,4.,5.)*color.xyz)+0.5;
		emission = 20.*color.xyz*exp(-50.*clamp(pow(abs(length(hfcol-vec3(0.2,0.5,0.75))),2.),0.,1.));
	}
	
	if (color_f.w < color.w) 
	{ 
		color = color_f; 
		pbr = vec2(0.2,0.35);
		if(FLAG_GLOW)
		{
			emission = 10.*color_f.xyz;
		}
	}
	
	if (color_m.w < color.w) 
	{ 
		color = color_m; 
		color.w = 1.;
		if(MARBLE_MODE <= 1)
		{
			pbr = vec2(1,0.35);
		}
		else
		{
			pbr = vec2(0.,0.35);
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

//calculate the normal using an already evaluated distance in one point
vec3 calcNormalA(in vec3 pos, in float h)
{
    vec4 e = vec4(0.0005,-0.0005, 1., -1);
    pos = pos - e.xxx;
    return normalize(e.zww*DE( pos + e.xyy ) + 
  			 		 e.wwz*DE( pos + e.yyx ) + 
			  		 e.wzw*DE( pos + e.yxy ) + 
              		 e.zzz*h );
}