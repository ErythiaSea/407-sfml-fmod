#version 330 core
out vec4 FragColor;

in vec4 gl_FragCoord;
in vec4 gl_TexCoord[1];

uniform sampler2D levelTexture;
uniform int screenBottom;

void main()
{
	vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
	vec4 texture_color = texture(levelTexture, gl_TexCoord[0].xy);

	if (texture_color != green) {
		FragColor = texture_color;
		return;
	}

	vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);
	vec4 purple = vec4(1.0, 0.0, 0.7, 1.0);

	float mixAmount = ((gl_FragCoord.y*2)/screenBottom);
	if (mixAmount <= 1) {
		FragColor = mix(green, blue, mixAmount);
	}
	else {
	FragColor = mix(blue, purple, mixAmount-1.0);
	}
}