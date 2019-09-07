vec3 bloom_radius(vec3 color, float exposure)
{
	color = clamp(color*exposure, 0, 30)/exposure;
	vec3 radius = 0.5f * tanh(2*(color * exposure - Camera.bloomtreshold)) + 0.5f;
	return radius;
}

vec3 bloom_treshold(vec3 color, float exposure)
{
	color = clamp(color*exposure, 0, 30)/exposure;
	vec3 mapped = Camera.bloomradius*(0.5f * tanh(2*(color * exposure - Camera.bloomtreshold)) + 0.5f);
	return color*mapped;
}