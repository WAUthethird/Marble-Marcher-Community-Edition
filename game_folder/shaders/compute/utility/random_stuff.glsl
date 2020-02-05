
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

vec3 ambient_sky_color(in vec3 pos)
{
	float y = pos.y;
	pos.xyz = normalize(vec3(1,0,0));
	return 0.5*sky_color(pos)*exp(-abs(y));
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
        al += ambient_sky_color(normalize(normal));
        if(i == 6 || i == 13) gi += ao*lcolor*shadow_march(vec4(pos, MIN_DIST), vec4(normalize(LIGHT_DIRECTION),0), 10., 0.3); // two GI samples
    }
    gi *= GIStrength/2.0;
    return gi + al * ao / float(AmbientLightSteps);
}
