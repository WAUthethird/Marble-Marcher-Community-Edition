vec3 bloom_treshold(vec3 color, float exposure)
{
	color = clamp(color*exposure, 0, 30)/exposure;
	vec3 mapped = Camera.bloomintensity*pow(color,vec3(2.f));
	return mapped;
}