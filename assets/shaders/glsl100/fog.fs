#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

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

uniform float litAmount;

void main()
{
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
    
    vec4 fogColor = vec4(0.5, 0.59, 0.67, 1.0);

    float lightf = mix((normal.y)*0.5 + .25, 1.0, litAmount);
    vec4 color = vec4(lightf, lightf, lightf, 1.0) * texelColor;
    float greyScale = dot(texelColor.rgb, vec3(0.299, 0.587, 0.114));

    float dist = length(viewPos - fragPosition) * 0.005 * fogDensity;
    float fogFactor = clamp(dist + (1.0 - greyScale * 2.0)*0.15, 0.0, 1.0);//-fragPosition.y * 0.0125;
    
    fogColor = mix(fogColor, fogColor, fogFactor);

    gl_FragColor = mix(color, fogColor, min(1.0, fogFactor));
}
