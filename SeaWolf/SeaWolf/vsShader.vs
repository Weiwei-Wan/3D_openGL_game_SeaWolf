attribute vec3 vertex_position;
attribute vec3 vertex_normal;
attribute vec2 vertex_texture;

varying vec3 LightIntensity;
varying vec2 TexCoord;

vec4 LightPosition = vec4 (0.0, -30.0, 0.0, 1.0); // Light position in world coords.
//vec3 Kd = vec3 (1.0, 0.0, 0.0); // green diffuse surface reflectance
vec3 Ld = vec3 (1.0, 1.0, 1.0); // Light source intensity

uniform vec3 color;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;
uniform mat4 Translation;

mat4 ModelViewMatrix = view * model;
mat3 NormalMatrix =  mat3(ModelViewMatrix[0].xyz, ModelViewMatrix[1].xyz, ModelViewMatrix[2].xyz);
// Convert normal and position to eye coords
// Normal in view space
vec3 tnorm = normalize( NormalMatrix * vertex_normal);
// Position in view space
vec4 eyeCoords = ModelViewMatrix * vec4(vertex_position,1.0);
//normalised vector towards the light source
vec3 s = normalize(vec3(LightPosition - eyeCoords));

void main() {
    // The diffuse shading equation, dot product gives us the cosine of angle between the vectors
    LightIntensity = Ld * color * max( dot( s, tnorm ), 0.0 );
    TexCoord = vertex_texture;
    // Convert position to clip coordinates and pass along
    gl_Position =  proj * view * model * (Translation * vec4(vertex_position,1.0));
}


  
