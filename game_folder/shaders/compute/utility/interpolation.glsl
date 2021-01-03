float sq(float x)
{
	return x*x;
}

float sq(vec2 x)
{
	return dot(x,x);
}

float sq(vec3 x)
{
	return dot(x,x);
}

float sq(vec4 x)
{
	return dot(x,x);
}


#define val(T, a, b, c) imageLoad(T, a+ivec2(b,c))

//better to use a sampler though
#define sample_quad(T, X) mat4(val(T, ivec2(X), 0, 0),val(T, ivec2(X), 1, 0),val(T, ivec2(X), 0, 1),val(T, ivec2(X), 1, 1))
#define coords_quad(X) ((vec4(1, 0, 1, 0) + vec4(-1, 1, -1, 1)*vec4((X).x - floor((X).x)))*(vec4(1, 1, 0, 0) + vec4(-1, -1, 1, 1)*vec4((X).y - floor((X).y))))

#define interp(T,X) (sample_quad(T,X)*coords_quad(X))

vec4 cubic(vec4 p0, vec4 p1, vec4 p2, vec4 p3, float x)
{
	return  p1 + 0.5 * x*(p2 - p0 + x*(2.0*p0 - 5.0*p1 + 4.0*p2 - p3 + x*(3.0*(p1 - p2) + p3 - p0)));
}

//2d interpolation that is aware of the 3d positions of our points
vec4 bilinear_surface_interp(mat4 quad, float td, float sz, vec2 coord)
{
	ivec2 ci = ivec2(coord);
	vec2 d = coord - floor(coord);
	
	float td1 = quad[0].w;
	float td2 = quad[1].w;
	float td3 = quad[2].w;
	float td4 = quad[3].w;
	
	float w1 = (1-d.x)*(1-d.y)/(sz*sz+(td-td1)*(td-td1));
	float w2 = (d.x)*(1-d.y)/(sz*sz+(td-td2)*(td-td2));
	float w3 = (1-d.x)*(d.y)/(sz*sz+(td-td3)*(td-td3));
	float w4 = (d.x)*(d.y)/(sz*sz+(td-td4)*(td-td4));
	
	return quad*vec4(w1,w2,w3,w4)/(w1+w2+w3+w4);
}

#define bilinear_surface(T, td, sz, coord) bilinear_surface_interp(sample_quad(T,coord),td,sz,coord)

vec4 gm(vec4 a)
{
	return vec4(pow(a.xyz,vec3(1.f/gamma_camera)),a.w);
}

//image part sampling defines
#define subImage(T, X, sub, subsize) imageLoad(T, clamp(X, ivec2(1), subsize-1) + sub*subsize)
#define storeSub(T, val, X, sub, subsize) imageStore(T, clamp(X, ivec2(1), subsize-1) + sub*subsize, val)
#define subQuad(T, X, sub, subsize) mat4(subImage(T, ivec2(X), sub, subsize),subImage(T, ivec2(X) + ivec2(1, 0), sub, subsize),subImage(T, ivec2(X) + ivec2(0, 1), sub, subsize),subImage(T, ivec2(X) + ivec2(1, 1), sub, subsize))
#define subInterp(T,X,sub,subsiz) (subQuad(T,X,sub,subsiz)*coords_quad(X))
