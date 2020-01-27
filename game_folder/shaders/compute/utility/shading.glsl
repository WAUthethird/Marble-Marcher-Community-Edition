#include<utility/ray_marching.glsl>

#define PI 3.14159265
#define AMBIENT_MARCHES 4
#define AMBIENT_COLOR 2*vec4(1,1,1,1)
#define LIGHT_ANGLE 0.08

uniform vec3 BACKGROUND_COLOR;
uniform vec3 LIGHT_DIRECTION;
uniform vec3 LIGHT_COLOR;
uniform bool SHADOWS_ENABLED; 

uniform float gamma_material;
uniform float gamma_sky;
uniform float gamma_camera;

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
	return abs(cubic(p0, p1, p2, p3, d.y));
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


float min_distance(layout (rgba32f) image2D T, vec3 cur, vec2 lastPos, int scale)
{
	ivec2 rp = ivec2(round(lastPos));
	float mdist = 1e10;
	for(int i = -scale; i <= scale; i++)
		for(int j = -scale; j <= scale; j++)
		{
			mdist = min(length(cur - imageLoad(T, rp+ivec2(i,j)).xyz), mdist);
		}
	return mdist;
}


///PBR functions 
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
///END PBR functions

const float Br = 0.0025;
const float Bm = 0.0003;
const float g =  0.9800;
const vec3 nitrogen = vec3(0.650, 0.570, 0.475);
const vec3 Kr = Br / pow(nitrogen, vec3(4.0));
const vec3 Km = Bm / pow(nitrogen, vec3(0.84));

vec3 sky_color(in vec3 pos)
{
	pos = normalize(pos);
	// Atmosphere Scattering
	vec3 fsun = LIGHT_DIRECTION;
	float brightnees = exp(-sqrt(pow(abs(min(5*(pos.y-0.01),0)),2)+0.));
	if(pos.y < 0)
	{
		pos.y = 0.;
		pos = normalize(pos);
	}
    float mu = dot(normalize(pos), normalize(fsun));
	
	vec3 extinction = mix(exp(-exp(-((pos.y + fsun.y * 4.0) * (exp(-pos.y * 16.0) + 0.1) / 80.0) / Br) * (exp(-pos.y * 16.0) + 0.1) * Kr / Br) * exp(-pos.y * exp(-pos.y * 8.0 ) * 4.0) * exp(-pos.y * 2.0) * 4.0, vec3(1.0 - exp(fsun.y)) * 0.2, -fsun.y * 0.2 + 0.5);
	vec3 sky_col = brightnees* 3.0 / (8.0 * 3.14) * (1.0 + mu * mu) * (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm) * extinction;
	sky_col = 0.4*clamp(sky_col,0.001,15.);
	return pow(sky_col,vec3(1.f/gamma_sky)); 
}

vec3 ambient_sky_color(in vec3 pos)
{
	float y = pos.y;
	pos.xyz = normalize(vec3(1,0,0));
	return 0.5*sky_color(pos)*exp(-abs(y));
}

vec4 ambient_occlusion(in vec4 pos, in vec4 norm, in vec4 dir)
{	
	vec3 pos0 = pos.xyz;
	
	float occlusion_angle = 0.;
	vec3 direction = normalize(norm.xyz);
	vec3 ambient_color = ambient_sky_color(norm.xyz);
	//step out
	pos.xyz += 0.01*dir.w*direction;
	//march in the direction of the normal
	for(int i = 0; i < AMBIENT_MARCHES; i++)
	{
		pos.xyz += pos.w*direction;
		pos.w = DE(pos.xyz);
		
		norm.w = length(pos0 - pos.xyz);
		occlusion_angle += clamp(pos.w/norm.w,0.,1.);
	}
	
	occlusion_angle /= float(AMBIENT_MARCHES); // average weighted by importance
	return vec4(ambient_color,1.)*(0.5-cos(3.14159265*occlusion_angle)*0.5);
}

//Global illumination approximation, loicvdb's shader https://www.shadertoy.com/view/3t3GWH used as reference
#define GIStrength .45
#define AOStrength .3
#define precision 1.01
#define AmbientLightSteps 14


vec3 ambient_light(vec3 pos, float d)
{
    vec3 pos0 = pos;
    float dist0 = DE(pos);
    vec3 normal = calcNormal(pos, d).xyz, gi = vec3(0.), al = vec3(0.0);
    float ao = 1., dist = dist0;
	vec3 lcolor = sky_color(LIGHT_DIRECTION);
    for(int i = 0; i < AmbientLightSteps; i++){
        float expectedDist = dist * (1. + .8);
        dist = max(DE(pos),MIN_DIST);
        float weight = AOStrength*(1.-float(i)/float(AmbientLightSteps));	//more weight to first samples
        ao *= pow(clamp(dist/max(max(expectedDist, d),MIN_DIST), 0., 1.0), weight);
        normal = normalize(calcNormalA(pos, dist) + (hash33(pos)-0.5));
        pos += normal/precision*dist; //slightly shorter to avoid artifacts
        al += ambient_sky_color(normal);
        if(i == 6 || i == 13) gi += ao*lcolor*shadow_march(vec4(pos, MIN_DIST), vec4(normalize(LIGHT_DIRECTION),0), 10., 0.3); // two GI samples
    }
    gi *= GIStrength/2.0;
    return gi + al * ao / float(AmbientLightSteps);
}


vec3 refraction(vec3 rd, vec3 n, float p) {
	float dot_nd = dot(rd, n);
	return p * (rd - dot_nd * n) + sqrt(1.0 - (p * p) * (1.0 - dot_nd * dot_nd)) * n;
}

vec3 lighting(vec4 color, vec2 pbr, vec4 pos, vec4 dir, vec4 norm, vec3 refl, vec3 refr, vec3 direct, vec3 GI) 
{
	vec3 albedo = color.xyz;
	albedo = pow(albedo,vec3(1.f/gamma_material)); //square it to make the fractals more colorfull 
	
	vec4 ambient_color = ambient_occlusion(pos, norm, dir);
	
	GI *= ambient_color.w;
	float metallic = pbr.x;
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
	
	//reflectance equation
	vec3 Lo = vec3(0.0);
	vec3 V = -dir.xyz;
	vec3 N = norm.xyz;
	
	if(!SHADOWS_ENABLED)
	{
		direct = ambient_color.xyz;
	}

	{ //light contribution
		float roughness = pbr.y;
		vec3 L = normalize(LIGHT_DIRECTION);
		vec3 H = normalize(V + L);
		vec3 radiance = direct;        
		
		// cook-torrance brdf
		float NDF = DistributionGGX(N, H, roughness);        
		float G   = GeometrySmith(N, V, L, roughness);      
		vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;	  
		
		vec3 numerator    = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular     = numerator / max(denominator, 0.001);  
			
		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);                
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
	
	{ 
		//light reflection, global illumination
		vec3 radiance = GI;               
		Lo += radiance*albedo; //remodulate albedo
	}

	if(color.w>0.5) // if marble
	{
		vec3 n = norm.xyz;
		vec3 q = dir.xyz - n*(2*dot(dir.xyz,n));
		//Combine for final marble color
		if(MARBLE_MODE == 0)
		{
			//glass
			vec3 F0 = vec3(0.03); 
			vec3 L = normalize(q);
			vec3 H = normalize(V + L);
			vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);  
			
			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			Lo += kS*refl + kD*refr;
		}
		else if(MARBLE_MODE > 0)
		{
			//metal
			vec3 F0 = vec3(0.6); 
			vec3 L = normalize(q);
			vec3 H = normalize(V + L);
			vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);  
			
			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			Lo += kS*refl;
		}
	}
	
	return Lo;
}

vec3 shading_simple(in vec4 pos, in vec4 dir, float fov, vec3 direct, float k)
{
	if(pos.w < max(16*fovray*dir.w, MIN_DIST))
	{
		//calculate the normal
		float error = 0.5*fov*dir.w;
		vec4 norm = calcNormal(pos.xyz, max(MIN_DIST, error)); 
		norm.xyz = normalize(norm.xyz);
		if(norm.w < -error)
		{
			return COL(pos.xyz).xyz;
		}
		else
		{
			//optimize color sampling 
			vec3 cpos = pos.xyz - pos.w*norm.xyz;
			
			vec4 color; vec2 pbr; vec3 emission;
			scene_material(cpos, color, pbr, emission);
			return lighting(color, pbr, pos, dir, norm, vec3(0), vec3(0), direct, vec3(k)); 
		}
	}
	else
	{
		return sky_color(dir.xyz);
	}
}

vec3 render_ray(in vec4 pos, in vec4 dir, float fov, float k)
{
	vec4 var = vec4(0,0,0,1);
	ray_march(pos, dir, var, fov); 
	vec3 direct = sky_color(LIGHT_DIRECTION)*shadow_march(pos, vec4(LIGHT_DIRECTION,0), 10., LIGHT_ANGLE);
	return shading_simple(pos, dir, fov, direct, k);
}

vec3 shading(in vec4 pos, in vec4 dir, float fov, layout (rgba32f) image2D  illuminationDirect, layout (rgba32f) image2D  illuminationGI,  layout (rgba32f) image2D  normals, vec3 RR)
{
	if(pos.w < max(2*fovray*dir.w, MIN_DIST))
	{
		//calculate the normal
		float error = 0.5*fov*dir.w;
		vec4 norm = calcNormal(pos.xyz, max(MIN_DIST, error)); 
		norm.xyz = normalize(norm.xyz);
		
		vec3 direct = bilinear_surface_enhance(illuminationDirect, normals, dir.w, RR.z, 0., norm.xyz, RR.xy).xyz;
		vec3 GI = bilinear_surface_enhance(illuminationGI, normals, dir.w, RR.z, 0.2, norm.xyz, RR.xy).xyz;
	
		if(norm.w < -error)
		{
			return COL(pos.xyz).xyz;
		}
		else
		{
			//optimize color sampling 
			vec3 cpos = pos.xyz - norm.w*norm.xyz;
			vec4 color; vec2 pbr; vec3 emission;
			scene_material(cpos, color, pbr, emission);
			vec3 refl = vec3(0);
			vec3 refr = vec3(0);
			if(color.w>0.5) // if marble
			{
				//Calculate refraction
				vec3 n = normalize(iMarblePos - cpos.xyz);
				vec3 q = refraction(dir.xyz, n, 1.0 / 1.5);
				vec3 p2 = pos.xyz + (dot(q, n) * 2. * iMarbleRad) * q;
				n = normalize(p2 - iMarblePos);
				q = (dot(q, dir.xyz) * 2.0) * q - dir.xyz;
				vec4 p_temp = vec4(p2+ n*fov*dir.w*2.5, 0);
				vec4 r_temp = vec4(q, dir.w);
				
				refr = render_ray(p_temp, r_temp, fov*1.5, length(GI));

				//Calculate reflection
				n = -normalize(iMarblePos - cpos.xyz);
				q = dir.xyz - n*(2*dot(dir.xyz,n));
				p_temp = vec4(pos.xyz + n*fov*dir.w*2., 0);
				r_temp = vec4(q, dir.w);
				
				refl = render_ray(p_temp, r_temp, fov*1.5, length(GI));
			}
			
			return lighting(color, pbr, vec4(cpos, pos.w), dir, norm, refl, refr, direct, GI); 
		}
	}
	else
	{
		return sky_color(dir.xyz);
	}
	
}

#define NEON_iterations 3 
#define NEON_marches 4

vec3 NEON_shading(in vec4 pos, in vec4 dir)
{
	vec3 color = vec3(0);
	for (int i = 0; i < NEON_iterations; i++) 
	{	
		for (int j = 0; j < NEON_marches; j++) 
		{	
			dir.w += pos.w;
			pos.xyz += pos.w*dir.xyz;
			pos.w = DE(pos.xyz);
		}
		//sample color at the closest point
		vec4 norm = calcNormal(pos.xyz, MIN_DIST); 
		vec3 cpos = pos.xyz - pos.w*norm.xyz;
		
		color += COL(cpos).xyz/(1+pos.w);
	}
	return color/NEON_iterations;
}

vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return (x*(a*x+b))/(x*(c*x+d)+e);
}

vec3 HDRmapping(vec3 color, float exposure)
{
	// Exposure tone mapping
    vec3 mapped = ACESFilm(color * exposure);
    // Gamma correction 
    return pow(mapped, vec3(1.0 / gamma_camera));
}