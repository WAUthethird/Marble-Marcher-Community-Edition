uniform float iFrame;

float hash(float p)
{  
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

vec2 hash22(vec2 p)
{
    vec2 res;
    res.x = hash(dot(p, vec2(1.,sqrt(2.))));
    res.y = hash(res.x);
    return res;
}

vec2 hash21(float p)
{
    vec2 res;
    res.x = hash(p);
    res.y = hash(res.x);
    return res;
}


vec3 hash31(float x)
{
    vec3 res;
    res.x = hash(x);
    res.y = hash(x+1.);
    res.z = hash(x+2.);
    return res;
}

//normally distributed random numbers
vec3 randn(float p)
{
    vec3 rand = hash31(p);
    vec3 box_muller = sqrt(-2.*min(log(max(rand.x,1e-6)), 0.))*vec3(sin(2.*PI*rand.y),cos(2.*PI*rand.y),sin(2.*PI*rand.z));
    return box_muller;
}

//uniformly inside a sphere
vec3 random_sphere(float p)
{
    return normalize(randn(p))*pow(abs(hash(p+1.)), 0.3333);
}

vec3 perlin31 (float p)
{
   float pi = floor(p);
   float pf = p - pi;
   return hash31(pi)*(1.-pf) +
          hash31(pi + 1.)*pf; 
}

vec3 perlin31(float p, float n)
{
    float frq = 1., amp = 1., norm = 0.;
    vec3 res = vec3(0.);
    for(float i = 0.; i < n; i++)
    {
        res += amp*perlin31(frq*p);
        norm += amp;
        frq *= 2.;
       // amp *= 1;
    }
    return res/norm;
}


vec2 perlin(vec2 p)
{
   vec2 pi = floor(p);
   vec2 pf = p - pi;
   vec2 a = vec2(0.,1.);
   return hash22(pi+a.xx)*(1.-pf.x)*(1.-pf.y) +
          hash22(pi+a.xy)*(1.-pf.x)*pf.y +
          hash22(pi+a.yx)*pf.x*(1.-pf.y) +
          hash22(pi+a.yy)*pf.x*pf.y;   
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
    vec2 pnoise = perlin(p*0.25+seed);
    
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