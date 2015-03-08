#version 130

in vec3 interpolateNormal;
in vec3 interpolateLightVector;
in vec2 textureCoord;

uniform sampler2D colorMap2D;

uniform float ambLight;     // for ambient lighting
uniform float diffLight;    // for diffuse lighting
uniform float material;
uniform float shininess;    // for specular lighting
uniform float noShine;

out vec4 finalColor;

void main()
{
	float ambContri = ambLight * material;


    float diff = max(0,dot(normalize(interpolateNormal),
	                normalize(interpolateLightVector)));
	float diffContri = diffLight * material * diff;


    vec3 ref = normalize(reflect(-normalize(interpolateLightVector), 
                                  normalize(interpolateNormal)));
	float spec = max(0, dot(normalize(interpolateNormal), ref));
	float specContri = pow(spec,shininess);
	if (noShine == 1)
	{
		specContri = 0;
	}

    float lightContri = ambContri + diffContri /*+ specContri*/;    // remove this comment
		
	finalColor = lightContri * texture(colorMap2D,textureCoord);	
    finalColor += vec4(specContri, specContri, specContri, 0);  // comment this out
}