vec3 bloom_treshold(vec3 color, float exposure)
{
	color = clamp(color*exposure, 0, 100)/exposure;
	vec3 mapped = Camera.bloomintensity*exposure*pow(color,vec3(2.));
	return mapped;
}