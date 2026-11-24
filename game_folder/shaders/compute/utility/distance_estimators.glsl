uniform sampler2D iTexture0;
uniform sampler2D iTexture1;

///Original MM distance estimators


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

//Early exit of the fractal loop.
//Every fold is an isometry, so once |p| is large the point can never come back,
//and once p.w is huge the distance is far below MIN_DIST anyway => the remaining
//iterations do not change the result. This makes FRACTAL_ITER effectively unlimited
//(the real number of iterations is ~log(1e11)/log(scale) at most).
#define FRAC_BAIL2 1e8
#define FRAC_WMAX 1e11
#define FRAC_WMIN 1e-20

//n-fold kaleidoscopic symmetry around the Y axis
void radialFold(inout vec4 p, int n)
{
	float sector = 6.28318530718/float(n);
	float a = atan(p.z, p.x);
	a = abs(mod(a + 0.5*sector, sector) - 0.5*sector);
	float r = length(p.xz);
	p.xz = r*vec2(cos(a), sin(a));
}

void fractal_step(inout vec4 p)
{
	if(SYM_RADIAL_INTERNAL > 1) radialFold(p, SYM_RADIAL_INTERNAL);
	p.xyz = mix(p.xyz, abs(p.xyz), SYM_MIRROR);
	p.xyz = iFracRot1*p.xyz;
	if(SYM_TYPE == 0) mengerFold(p);
	else if(SYM_TYPE == 1) sierpinskiFold(p);
	else if(SYM_TYPE == 2) p.xy += min(p.x - p.y, 0.0)*mp;
	p.xyz = iFracRot2*p.xyz;
	p *= iFracScale;
	p.xyz += iFracShift;
}

#define FRAC_EXIT (dot(p.xyz,p.xyz) > FRAC_BAIL2 || p.w > FRAC_WMAX || p.w < FRAC_WMIN)

float de_fractal(vec4 p)
{
	if(SYM_DEFAULT)
	{
		//fast path for the original menger symmetry: no per iteration branches
		for (int i = 0; i < FRACTAL_ITER; ++i) {
			p.xyz = iFracRot1*abs(p.xyz);
			mengerFold(p);
			p.xyz = iFracRot2*p.xyz;
			p *= iFracScale;
			p.xyz += iFracShift;
			if(FRAC_EXIT) break;
		}
		return de_box(p, vec3(6.0));
	}
	if(SYM_RADIAL_GLOBAL > 1) radialFold(p, SYM_RADIAL_GLOBAL);
	for (int i = 0; i < FRACTAL_ITER; ++i) {
		fractal_step(p);
		if(FRAC_EXIT) break;
	}
	return de_box(p, vec3(6.0));
}

//surface color orbit trap + glow orbit trap in one pass
vec4 col_fractal_glow(vec4 p, out vec3 gorbit)
{
	vec3 orbit = vec3(0.0);
	gorbit = vec3(0.0);
	if(SYM_RADIAL_GLOBAL > 1) radialFold(p, SYM_RADIAL_GLOBAL);
	for (int i = 0; i < FRACTAL_ITER; ++i) {
		fractal_step(p);
		orbit = max(orbit, p.xyz*iFracCol);
		gorbit = max(gorbit, p.xyz*GLOW_PATTERN);
		if(FRAC_EXIT) break;
	}
	return vec4(orbit, de_box(p, vec3(6.0)));
}

vec4 col_fractal(vec4 p)
{
	vec3 orbit = vec3(0.0);
	if(SYM_RADIAL_GLOBAL > 1) radialFold(p, SYM_RADIAL_GLOBAL);
	for (int i = 0; i < FRACTAL_ITER; ++i) {
		fractal_step(p);
		orbit = max(orbit, p.xyz*iFracCol);
		if(FRAC_EXIT) break;
	}
	return vec4(orbit, de_box(p, vec3(6.0)));
}

//marble, squashed into an ellipsoid when the rubber ball hits something
float de_marble(vec4 p)
{
	if(iMarbleSquash.w == 0.0)
	{
		return de_sphere(p - vec4(iMarblePos, 0), iMarbleRad);
	}
	float s = iMarbleSquash.w;
	vec3 n = iMarbleSquash.xyz;
	vec3 q = p.xyz - (iMarblePos + n*(iMarbleRad*s));
	float a = 1.0 - s;                    //scale along the contact normal
	float b = inversesqrt(max(a, 0.05));  //volume preserving bulge
	float qn = dot(q, n);
	vec3 qs = n*(qn/a) + (q - n*qn)/b;
	float d = (length(qs) - iMarbleRad)*min(a, b);
	if(iMarbleLumps > 0.0)
	{
		//crushed metal: lumpy dumpling surface
		vec3 k = q*(18.0/iMarbleRad);
		d += iMarbleLumps*iMarbleRad*0.06*sin(k.x)*sin(k.y + 1.3)*sin(k.z + 2.1);
		d *= 0.7;
	}
	return d/p.w;
}

//glass shards of a crushed marble
vec3 quat_rotate_inv(vec4 q, vec3 v)
{
	vec3 u = -q.xyz;
	return v + 2.0*cross(u, cross(u, v) + q.w*v);
}

float de_shard(vec4 p, int i)
{
	vec3 q = quat_rotate_inv(iShardRot[i], p.xyz - iShardPos[i].xyz);
	q = abs(q);
	return (q.x + 2.5*q.y + q.z*0.8 - iShardPos[i].w*0.5)*0.36/p.w;
}

float de_shards(vec4 p, float best)
{
	if (length(p.xyz - iShardBound.xyz) - iShardBound.w > best) return best;
	float d = best;
	for (int i = 0; i < iShardCount; i++)
	{
		if (length(p.xyz - iShardPos[i].xyz) - iShardPos[i].w > d) continue;
		d = min(d, de_shard(p, i));
	}
	return d;
}

vec4 col_marble(vec4 p)
{
	vec4 col = vec4(0.5, 0.5, 0.5, de_marble(p));
	return col;
}

float de_flag_at(vec4 p, vec3 fpos)
{
	vec3 f_pos = fpos + vec3(1.5, 4, 0)*iFlagScale;
	vec4 p_s = p/iMarbleRad;
	vec4 d_pos = p - vec4(f_pos, 0);
	vec4 caps_pos = p - vec4(fpos + vec3(0, iFlagScale*2.4, 0), 0);
	//animated flag woooo
	float speed = 14;
	float oscillation = sin(8*p_s.x - 1*p_s.y - speed*time) + 0.4*sin(11*p_s.x + 2*p_s.y - 1.2*speed*time) + 0.15*sin(20*p_s.x - 5*p_s.y -1.4*speed*time);
	//scale the flag displacement amplitude by the distance from the flagpole
	float d = 0.4*de_box(d_pos + caps_pos.x*vec4(0,(0.02+ caps_pos.x* 0.5+0.01*oscillation),0.04*oscillation,0), vec3(1.5, 0.8, 0.005)*iMarbleRad);
	d = min(d, de_capsule(caps_pos, iMarbleRad*2.4, iMarbleRad*0.05));
	return d;
}

float de_flag(vec4 p)
{
	return de_flag_at(p, iFlagPos);
}

//state: 0 locked (not reachable yet), 1 current target, 2 collected
vec4 col_flag_at(vec4 p, vec3 fpos, float state)
{
	vec3 f_pos = fpos + vec3(1.5, 4, 0)*iFlagScale;
	vec4 d_pos = p - vec4(f_pos, 0);
	vec3 fsize = vec3(1.5, 0.8, 0.08)*iMarbleRad;
	vec4 caps_pos = p - vec4(fpos + vec3(0, iFlagScale*2.4, 0), 0);
	float d1 = de_box(d_pos, fsize);
	float d2 = de_capsule(caps_pos, iMarbleRad*2.4, iMarbleRad*0.18);
	if (d1 < d2) {
		vec2 texture_coord = d_pos.xy*vec2(0.5,-0.48)/fsize.xy + vec2(0.5,0.5) - 0.5*vec2(0,caps_pos.x*(0.02+ caps_pos.x* 0.5))/fsize.xy;
		vec3 flagcolor = texture(iTexture0, texture_coord).xyz;
		if(state < 0.5) flagcolor = vec3(dot(flagcolor, vec3(0.3,0.59,0.11)))*0.25;
		else if(state > 1.5) flagcolor = mix(flagcolor, vec3(0.1,0.9,0.2), 0.65);
		return vec4(flagcolor, d1);
	} else {
		vec3 pole = vec3(0.9, 0.9, 0.1);
		if(state < 0.5) pole = vec3(0.25);
		else if(state > 1.5) pole = vec3(0.1, 0.85, 0.2);
		return vec4(pole, d2);
	}
}

vec4 col_flag(vec4 p)
{
	return col_flag_at(p, iFlagPos, iGoalState);
}

//checkpoint: thin pole with a slowly spinning crystal on top
float de_checkpoint(vec4 p, vec3 cpos, out bool gem)
{
	vec4 caps_pos = p - vec4(cpos + vec3(0, iFlagScale*2.4, 0), 0);
	float d = de_capsule(caps_pos, iMarbleRad*2.4, iMarbleRad*0.07);
	vec3 q = p.xyz - (cpos + vec3(0, iFlagScale*5.6, 0));
	float a = time*1.5;
	q.xz = mat2(cos(a), sin(a), -sin(a), cos(a))*q.xz;
	q = abs(q);
	float oct = (q.x + q.y*0.7 + q.z - iMarbleRad*0.9)*0.57735/p.w;
	gem = oct < d;
	return min(d, oct);
}

//distance to all extra flags and checkpoints; objects whose bounding sphere
//is further away than the current best distance are skipped (cheap)
float de_extra(vec4 p, float best)
{
	float d = best;
	for (int i = 0; i < iFlagCount; i++)
	{
		vec3 c = iFlagArr[i] + vec3(0, 2.4*iFlagScale, 0);
		if (length(p.xyz - c) - 4.2*iMarbleRad < d)
			d = min(d, de_flag_at(p, iFlagArr[i]));
	}
	for (int i = 0; i < iCPCount; i++)
	{
		vec3 c = iCPArr[i] + vec3(0, 3.2*iFlagScale, 0);
		if (length(p.xyz - c) - 4.2*iMarbleRad < d)
		{
			bool gem;
			d = min(d, de_checkpoint(p, iCPArr[i], gem));
		}
	}
	return d;
}

//color of the nearest extra object, w = distance
vec4 col_extra(vec4 p, out vec3 emission)
{
	vec4 best = vec4(0, 0, 0, 1e10);
	emission = vec3(0);
	for (int i = 0; i < iFlagCount; i++)
	{
		vec3 c = iFlagArr[i] + vec3(0, 2.4*iFlagScale, 0);
		if (length(p.xyz - c) - 4.2*iMarbleRad > best.w) continue;
		vec4 cf = col_flag_at(p, iFlagArr[i], iFlagState[i]);
		if (cf.w < best.w)
		{
			best = cf;
			emission = (FLAG_GLOW && iFlagState[i] > 0.5 && iFlagState[i] < 1.5) ? 10.*cf.xyz : vec3(0);
		}
	}
	for (int i = 0; i < iCPCount; i++)
	{
		vec3 c = iCPArr[i] + vec3(0, 3.2*iFlagScale, 0);
		if (length(p.xyz - c) - 4.2*iMarbleRad > best.w) continue;
		bool gem;
		float d = de_checkpoint(p, iCPArr[i], gem);
		if (d < best.w)
		{
			bool cp_on = iCPState[i] > 0.5 && iCPState[i] < 1.5;
			bool cp_next = iCPState[i] > 1.5;
			vec3 col = gem ? (cp_on ? vec3(0.15, 1.0, 0.3) : (cp_next ? vec3(1.0, 0.8, 0.1) : vec3(0.25, 0.3, 0.45))) : vec3(0.8);
			best = vec4(col, d);
			emission = (gem && cp_on) ? 6.*col : ((gem && cp_next) ? (3.0 + 2.5*sin(time*5.0))*col : vec3(0));
		}
	}
	return best;
}

float de_scene(vec3 pos)
{
	vec4 p = vec4(pos,1.f);
	float d = de_fractal(p);
	//bounding spheres: the exact marble / flag DE is only needed when they can be the closest object
	if(length(pos - iMarblePos) - 2.5*iMarbleRad < d) d = min(d, de_marble(p));
	if(length(pos - (iFlagPos + vec3(0, 2.4*iFlagScale, 0))) - 4.2*iMarbleRad < d) d = min(d, de_flag(p));
	if(iFlagCount + iCPCount > 0) d = de_extra(p, d);
	if(iShardCount > 0) d = de_shards(p, d);
	return d;
}

vec4 col_scene(vec3 pos)
{
	vec4 p = vec4(pos,1.f);
	vec4 col = col_fractal(p);
	vec4 col_f = col_flag(p);
	if (col_f.w < col.w) { col = col_f; }
	if(iFlagCount + iCPCount > 0)
	{
		vec3 e;
		vec4 col_e = col_extra(p, e);
		if (col_e.w < col.w) { col = col_e; }
	}
	if(iShardCount > 0)
	{
		float ds = de_shards(p, col.w);
		if (ds < col.w) col = vec4(iShardColor, ds);
	}
	vec4 col_m = col_marble(p);
	if (col_m.w < col.w) {
		if(MARBLE_MODE == 3) return vec4(vec3(0.02), 0.0);
		if(MARBLE_MODE == 4) return vec4(vec3(0.6), 0.0);
		return vec4(col_m.xyz, 1.0);
	}
	return vec4(min(col.xyz,1), 0.0);
}

float glow_mask(vec3 c)
{
	vec3 hfcol = 0.5*sin(vec3(3.,4.,5.)*c)+0.5;
	return exp(-50.*clamp(pow(abs(length(hfcol-vec3(0.2,0.5,0.75))),2.),0.,1.));
}

void scene_material(vec3 pos, inout vec4 color, inout vec2 pbr, inout vec3 emission)
{
	//DE_count = DE_count+1;
	vec4 p = vec4(pos,1.f);
	emission = vec3(0.);

	vec3 gorbit;
	color = col_fractal_glow(p, gorbit);
	vec4 color_f = col_flag(p);
	vec4 color_m = col_marble(p);

	pbr = vec2(PBR_METALLIC, PBR_ROUGHNESS);
	float reflection = 0;

	if(FRACTAL_GLOW)
	{
		if(GLOW_CUSTOM)
		{
			//glow color is independent from the surface color
			emission = 20.*GLOW_BRIGHTNESS*GLOW_BEAT*GLOW_COLOR*glow_mask(gorbit);
		}
		else
		{
			//legacy glow (global setting only)
			emission = 20.*color.xyz*glow_mask(color.xyz);
		}
	}

	if (color_f.w < color.w)
	{
		color = color_f;
		pbr = vec2(0.2,0.35);
		emission = vec3(0);
		if(FLAG_GLOW && iGoalState > 0.5 && iGoalState < 1.5)
		{
			emission = 10.*color_f.xyz;
		}
	}

	if(iFlagCount + iCPCount > 0)
	{
		vec3 e;
		vec4 color_e = col_extra(p, e);
		if (color_e.w < color.w)
		{
			color = color_e;
			pbr = vec2(0.2,0.35);
			emission = e;
		}
	}

	if(iShardCount > 0)
	{
		float ds = de_shards(p, color.w);
		if (ds < color.w)
		{
			color = vec4(iShardColor, ds);
			pbr = vec2(0.9, 0.08); //shiny glass/ceramic splinters
			emission = vec3(0);
		}
	}

	if (color_m.w < color.w)
	{
		color = color_m;
		emission = vec3(0);
		if(MARBLE_MODE == 4)
		{
			//crushed metal dumpling: rendered with normal lighting (correct normals)
			color = vec4(vec3(0.62), 0.);
			pbr = vec2(1.0, 0.3);
		}
		else if(MARBLE_MODE == 3)
		{
			//rubber ball: matte black
			color = vec4(vec3(0.02), 0.);
			pbr = vec2(0., 0.9);
		}
		else
		{
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