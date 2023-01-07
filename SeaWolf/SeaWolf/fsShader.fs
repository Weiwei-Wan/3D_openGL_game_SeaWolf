varying vec3 LightIntensity;
varying vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    gl_FragColor = vec4 (LightIntensity, 1.0); // texture2D(ourTexture, TexCoord);// * vec4 (LightIntensity, 1.0);
}

