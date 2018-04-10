uniform vec2 lightLocation;
uniform vec4 lightColor;

void main() {
	float distance = length(lightLocation - gl_FragCoord.xy);
	float attenuation = 40.0 / distance;
	vec4 color = vec4(attenuation, attenuation, attenuation, pow(attenuation, 0.25)) * lightColor;

	gl_FragColor = color;
}