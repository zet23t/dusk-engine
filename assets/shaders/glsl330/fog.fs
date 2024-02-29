#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float litAmount;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct MaterialProperty {
    vec3 color;
    int useSampler;
    sampler2D sampler;
};

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;
uniform float fogDensity;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
    
    vec4 fogColor = vec4(0.5, 0.59, 0.67, 1.0);

    float dist = length(viewPos - fragPosition);
    float fogFactor = -fragPosition.y * 0.0125;
    float greyScale = dot(texelColor.rgb, vec3(0.299, 0.587, 0.114));
    fogColor = mix(vec4(greyScale, greyScale, greyScale, 1.0), fogColor, fogFactor);

    float lightf = mix((normal.y)*0.5 + .25, 1.0, litAmount);
    vec4 color = vec4(lightf, lightf, lightf, 1.0) * texelColor;
    finalColor = mix(color, fogColor, min(1, fogFactor));
    // finalColor = texelColor;
    // finalColor = vec4(0, -fragPosition.y*.01, 0.0, 1.0);
}
