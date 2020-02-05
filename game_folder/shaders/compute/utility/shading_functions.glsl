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

vec3 refraction(vec3 rd, vec3 n, float p) {
	float dot_nd = dot(rd, n);
	return p * (rd - dot_nd * n) + sqrt(1.0 - (p * p) * (1.0 - dot_nd * dot_nd)) * n;
}

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

vec3 BRDF(vec3 Ri, vec3 Ro, vec3 N, vec3 albedo, vec2 pbr)
{
	vec3 F0 = mix(vec3(0.04), albedo, pbr.x);
	vec3 H = normalize(Ri + Ro);
	
	// cook-torrance brdf
	float NDF = DistributionGGX(N, H, pbr.y);        
	float G   = GeometrySmith(N, Ri, Ro, pbr.y);      
	vec3 F    = fresnelSchlick(max(dot(H, Ri), 0.0), F0);       
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - pbr.x;	  
	
	vec3 numerator    = NDF * G * F;
	float denominator = 4.0 * max(dot(N, Ri), 0.0) * max(dot(N, Ro), 0.0);
	vec3 specular     = numerator / max(denominator, 0.001);  
		                
	return (kD * albedo / PI + specular);
}