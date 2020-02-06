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


//better to use a sampler though
vec4 interp(layout (rgba32f) image2D text, vec2 coord)
{
	//coord += vec2(0.5);
	ivec2 ci = ivec2(coord);
	vec2 d = coord - floor(coord);
	return (imageLoad(text, ci)*(1-d.x)*(1-d.y) +
		   imageLoad(text, ci+ivec2(1,0))*d.x*(1-d.y) +
		   imageLoad(text, ci+ivec2(0,1))*(1-d.x)*d.y +
		   imageLoad(text, ci+ivec2(1,1))*d.x*d.y);
}

vec4 cubic(vec4 p0, vec4 p1, vec4 p2, vec4 p3, float x)
{
	return  p1 + 0.5 * x*(p2 - p0 + x*(2.0*p0 - 5.0*p1 + 4.0*p2 - p3 + x*(3.0*(p1 - p2) + p3 - p0)));
}

vec4 val(layout (rgba32f) image2D T, ivec2 a, int b, int c)
{
	return imageLoad(T, a+ivec2(b,c));
}

vec4 interp_bicubic(layout (rgba32f) image2D T, vec2 coord)
{
	ivec2 i = ivec2(coord);
	vec2 d = coord - floor(coord);
	vec4 p0 = cubic(val(T, i, -1,-1), val(T, i, 0,-1), val(T, i, 1,-1), val(T, i, 2,-1), d.x);
	vec4 p1 = cubic(val(T, i, -1, 0), val(T, i, 0, 0), val(T, i, 1, 0), val(T, i, 2, 0), d.x);
	vec4 p2 = cubic(val(T, i, -1, 1), val(T, i, 0, 1), val(T, i, 1, 1), val(T, i, 2, 1), d.x);
	vec4 p3 = cubic(val(T, i, -1, 2), val(T, i, 0, 2), val(T, i, 1, 2), val(T, i, 2, 2), d.x);
	return abs(cubic(p0, p1, p2, p3, d.y));
}

vec4 bicubic_surface(layout (rgba32f) image2D T, float td, float sz, vec2 coord)
{
	ivec2 ic = ivec2(coord);
	vec2 d = coord - floor(coord);
	//load data
	vec4 data[4][4];
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
		{
			data[i][j] = val(T, ic, i-1, j-1);
		}
	
	//3d distance interpolation
	vec4 datai[4][4];
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
		{
			vec4 sum = vec4(0);
			float k = 0.;
			for(int ii = 0; ii < 4; ii++)
				for(int jj = 0; jj < 4; jj++)
				{
					float c = 1./( sq(td - data[ii][jj].w) + sq(sz)*(sq(vec2(i-ii,j-jj)) + 0.001) );
					sum += data[ii][jj]*c;
					k += c;
				}
			datai[i][j] = sum/k;
		}
	
	//2d bicubic
	vec4 p0 = cubic(datai[0][0], datai[1][0], datai[2][0], datai[3][0], d.x);
	vec4 p1 = cubic(datai[0][1], datai[1][1], datai[2][1], datai[3][1], d.x);
	vec4 p2 = cubic(datai[0][2], datai[1][2], datai[2][2], datai[3][2], d.x);
	vec4 p3 = cubic(datai[0][3], datai[1][3], datai[2][3], datai[3][3], d.x);
	return cubic(p0, p1, p2, p3, d.y);
}

vec4 interp_sharp(layout (rgba32f) image2D text, vec2 coord, float sharpness)
{
	//coord += vec2(0.5);
	ivec2 ci = ivec2(coord);
	vec2 d = coord - floor(coord);
	float b0 = tanh(0.5*sharpness);
	vec2 k = (tanh(sharpness*(d - 0.5))+b0)*0.5/b0;
	vec4 r1 = mix(imageLoad(text, ci), imageLoad(text, ci+ivec2(1,0)), k.x);
	vec4 r2 = mix(imageLoad(text, ci+ivec2(0,1)), imageLoad(text, ci+ivec2(1,1)), k.x);
	vec4 c = mix(r1, r2, k.y);
	return c;
}

//2d interpolation that is aware of the 3d positions of our points
vec4 bilinear_surface(layout (rgba32f) image2D text, float td, float sz, vec2 coord)
{
	ivec2 ci = ivec2(coord);
	vec2 d = coord - floor(coord);
	
	vec4 A1 = imageLoad(text, ci);
	vec4 A2 = imageLoad(text, ci+ivec2(1,0));
	vec4 A3 = imageLoad(text, ci+ivec2(0,1));
	vec4 A4 = imageLoad(text, ci+ivec2(1,1));
	
	float td1 = A1.w;
	float td2 = A2.w;
	float td3 = A3.w;
	float td4 = A4.w;
	
	float w1 = (1-d.x)*(1-d.y)/(sz*sz+(td-td1)*(td-td1));
	float w2 = (d.x)*(1-d.y)/(sz*sz+(td-td2)*(td-td2));
	float w3 = (1-d.x)*(d.y)/(sz*sz+(td-td3)*(td-td3));
	float w4 = (d.x)*(d.y)/(sz*sz+(td-td4)*(td-td4));
	
	//a fix for gamma ruining the interpolation
	return pow((pow(A1,vec4(1.f/gamma_camera))*w1 + pow(A2,vec4(1.f/gamma_camera))*w2 + pow(A3,vec4(1.f/gamma_camera))*w3 + pow(A4,vec4(1.f/gamma_camera))*w4)/(w1+w2+w3+w4),vec4(gamma_camera));
}


vec4 gm(vec4 a)
{
	return vec4(pow(a.xyz,vec3(1.f/gamma_camera)),a.w);
}


//2d interpolation which modulates the color by the normal
vec4 bilinear_surface_enhance(layout (rgba32f) image2D T, layout (rgba32f) image2D N, float td, float sz, float enh, vec3 normal, vec2 coord)
{
	ivec2 ci = ivec2(coord);
	vec2 d = coord - floor(coord);
	
	vec4 A1 = gm(imageLoad(T, ci));
	vec4 A2 = gm(imageLoad(T, ci+ivec2(1,0)));
	vec4 A3 = gm(imageLoad(T, ci+ivec2(0,1)));
	vec4 A4 = gm(imageLoad(T, ci+ivec2(1,1)));
	
	vec3 N1 = imageLoad(N, ci).xyz;
	vec3 N2 = imageLoad(N, ci+ivec2(1,0)).xyz;
	vec3 N3 = imageLoad(N, ci+ivec2(0,1)).xyz;
	vec3 N4 = imageLoad(N, ci+ivec2(1,1)).xyz;
	
	////color(normal) remodulation
	vec3 Navg = 0.25*(N2+N4+N1+N3);
	vec4 Aavg = 0.25*(A1+A2+A3+A4);
	vec3 dNx = 0.5*(N2+N4-N1-N3);
	vec3 dNy = 0.5*(N3+N4-N1-N2);
	
	vec2 D = vec2(dot(Camera.dirx, dNx),dot(Camera.diry, dNy));
	
	vec3 dAx = 0.5*(A2.xyz+A4.xyz-A1.xyz-A3.xyz)*D.x/(sq(D.x) + 0.01);
	vec3 dAy = 0.5*(A3.xyz+A4.xyz-A1.xyz-A2.xyz)*D.y/(sq(D.y) + 0.01);
	
	vec3 dN = normal - Navg;
	vec3 colormod = Aavg.xyz*tanh(80*(dot(Camera.dirx, dN)*dAx + dot(Camera.diry, dN)*dAy)/Aavg.xyz);
	////
	
	float w1 = (1-d.x)*(1-d.y)/(sq(sz)*(1. + 50.*sq(normal - N1))+sq(td-A1.w));
	float w2 = (d.x)*(1-d.y)/(sq(sz)*(1. + 50.*sq(normal - N2))+sq(td-A2.w));
	float w3 = (1-d.x)*(d.y)/(sq(sz)*(1. + 50.*sq(normal - N3))+sq(td-A3.w));
	float w4 = (d.x)*(d.y)/(sq(sz)*(1. + 50.*sq(normal - N4))+sq(td-A4.w));
	
	//a fix for gamma ruining the interpolation
	return pow((A1*w1 + A2*w2 + A3*w3 + A4*w4)/(w1+w2+w3+w4) + enh*vec4(colormod,0.),vec4(gamma_camera));
}

//image part sampling functions
vec4 subImage(layout (rgba32f) image2D T, ivec2 coord, ivec2 sub, ivec2 subsize)
{
	ivec2 imgsize = imageSize(T);
	ivec2 subnum = imgsize/subsize;
	return imageLoad(T, clamp(coord, ivec2(1), subsize-1) + sub*subsize);
}

void storeSub(layout (rgba32f) image2D T, vec4 val, ivec2 coord, ivec2 sub, ivec2 subsize)
{
	ivec2 imgsize = imageSize(T);
	ivec2 subnum = imgsize/subsize;
	imageStore(T, clamp(coord, ivec2(1), subsize-1) + sub*subsize, val);
}

vec4 subInterp(layout (rgba32f) image2D T, vec2 coord, ivec2 sub, ivec2 subsize)
{
	//coord += vec2(0.5);
	ivec2 ci = ivec2(coord);
	vec2 d = coord - floor(coord);
	
	return (subImage(T, ci, sub, subsize)*(1-d.x)*(1-d.y) +
		    subImage(T, ci+ivec2(1,0), sub, subsize)*d.x*(1-d.y) +
		    subImage(T, ci+ivec2(0,1), sub, subsize)*(1-d.x)*d.y +
		    subImage(T, ci+ivec2(1,1), sub, subsize)*d.x*d.y);
}