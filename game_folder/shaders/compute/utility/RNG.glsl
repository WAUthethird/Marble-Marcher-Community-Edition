// HASH STUFF

// Hash without Sine
// MIT License...
/* Copyright (c)2014 David Hoskins.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

//----------------------------------------------------------------------------------------
//  1 out, 1 in...
float hash(float p)
{
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

//----------------------------------------------------------------------------------------
//  1 out, 2 in...
float hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

//----------------------------------------------------------------------------------------
//  1 out, 3 in...
float hash13(vec3 p3)
{
	p3  = fract(p3 * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

//----------------------------------------------------------------------------------------
//  2 out, 1 in...
vec2 hash21(float p)
{
	vec3 p3 = fract(vec3(p) * vec3(.1031, .1030, .0973));
	p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xx+p3.yz)*p3.zy);

}

//----------------------------------------------------------------------------------------
///  2 out, 2 in...
vec2 hash22(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return fract((p3.xx+p3.yz)*p3.zy);

}

//----------------------------------------------------------------------------------------
///  2 out, 3 in...
vec2 hash23(vec3 p3)
{
	p3 = fract(p3 * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return fract((p3.xx+p3.yz)*p3.zy);
}

//----------------------------------------------------------------------------------------
//  3 out, 1 in...
vec3 hash31(float p)
{
   vec3 p3 = fract(vec3(p) * vec3(.1031, .1030, .0973));
   p3 += dot(p3, p3.yzx+33.33);
   return fract((p3.xxy+p3.yzz)*p3.zyx); 
}


//----------------------------------------------------------------------------------------
///  3 out, 2 in...
vec3 hash32(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+33.33);
    return fract((p3.xxy+p3.yzz)*p3.zyx);
}

//----------------------------------------------------------------------------------------
///  3 out, 3 in...
vec3 hash33(vec3 p3)
{
	p3 = fract(p3 * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+33.33);
    return fract((p3.xxy + p3.yxx)*p3.zyx);

}

//----------------------------------------------------------------------------------------
// 4 out, 1 in...
vec4 hash41(float p)
{
	vec4 p4 = fract(vec4(p) * vec4(.1031, .1030, .0973, .1099));
    p4 += dot(p4, p4.wzxy+33.33);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);
    
}

//----------------------------------------------------------------------------------------
// 4 out, 2 in...
vec4 hash42(vec2 p)
{
	vec4 p4 = fract(vec4(p.xyxy) * vec4(.1031, .1030, .0973, .1099));
    p4 += dot(p4, p4.wzxy+33.33);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);

}

//----------------------------------------------------------------------------------------
// 4 out, 3 in...
vec4 hash43(vec3 p)
{
	vec4 p4 = fract(vec4(p.xyzx)  * vec4(.1031, .1030, .0973, .1099));
    p4 += dot(p4, p4.wzxy+33.33);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);
}

//----------------------------------------------------------------------------------------
// 4 out, 4 in...
vec4 hash44(vec4 p4)
{
	p4 = fract(p4  * vec4(.1031, .1030, .0973, .1099));
    p4 += dot(p4, p4.wzxy+33.33);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);
}


//normally distributed random numbers
vec3 randn(float p)
{
    vec4 rand = hash41(p);
    vec3 box_muller = sqrt(-2.*log(max(vec3(rand.x,rand.x,rand.z),1e-8)))*vec3(sin(2.*PI*rand.y),cos(2.*PI*rand.y),sin(2.*PI*rand.w));
    return box_muller;
}

//uniformly inside a sphere
vec3 random_sphere(float p)
{
    return normalize(randn(p))*pow(hash(p+85.67),0.333333);
}

vec3 cosdistr(vec3 dir, float seed)
{
    vec3 rand_dir = normalize(randn(seed*SQRT2));
    vec3 norm_dir = normalize(rand_dir - dot(dir,rand_dir)*dir);
    float u = hash(seed);
    return normalize(dir*sqrt(u) + norm_dir*sqrt(1.-u));
}


vec4 perlin_octave(vec2 p)
{
   vec2 pi = floor(p);
   vec2 pf = p - pi;
   vec2 pfc = 0.5 - 0.5*cos(pf*PI);
   vec2 a = vec2(0.,1.);
   
   vec4 a00 = hash42(pi+a.xx);
   vec4 a01 = hash42(pi+a.xy);
   vec4 a10 = hash42(pi+a.yx);
   vec4 a11 = hash42(pi+a.yy);
   
   vec4 i1 = mix(a00, a01, pfc.y);
   vec4 i2 = mix(a10, a11, pfc.y);
   
   return mix(i1, i2, pfc.x);  
}

mat2 rotat = mat2(cos(0.5), -sin(0.5), sin(0.5), cos(0.5));

vec4 perlin4(vec2 p)
{
	float a = 1.;
	vec4 res = vec4(0.);
	for(int i = 0; i < PERLIN_SCALE; i++)
	{
		res += a*(perlin_octave(p)-0.5);
        //inverse perlin
		p *= 0.6*rotat;
		a *= 1.2;
	}
	return res;
}

float singrid(vec2 p, float angle)
{
    return 0.5*(sin(cos(angle)*p.x + sin(angle)*p.y)*sin(-sin(angle)*p.x + cos(angle)*p.y) + 1.);
}

//technically this is not a blue noise, but a single freqency noise, the spectrum should look like a gaussian peak around a frequency
float blue(vec2 p, float seed)
{ 
    seed = 100.*hash(seed);
    vec2 shift = 20.*hash21(seed);
    p += shift;
    vec2 pnoise = perlin4(p*0.25+seed).xy;
    
    //bilinear interpolation between sin grids
    return singrid(p,0.)*(pnoise.x*pnoise.y+(1.-pnoise.x)*(1.-pnoise.y)) +
           singrid(p,3.14159*0.33*2.)*(1.-pnoise.x)*pnoise.y +
           singrid(p,3.14159*0.66*2.)*(1.-pnoise.y)*pnoise.x;
}

vec3 blue3(vec2 p, float seed)
{
    vec3 res;
    res.x = blue(p, sin(seed));
    res.y = blue(p, sin(2.*seed));
    res.z = blue(p, sin(3.*seed));
    return res;
}