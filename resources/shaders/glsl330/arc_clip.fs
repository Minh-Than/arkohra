#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float clipZ;   // world Z to do the clipping

out vec4 finalColor;

void main()
{
    if (fragPosition.z < clipZ) discard;   // cut everything past the current playhead position

    vec4 texelColor = texture(texture0, fragTexCoord);
    finalColor = texelColor * colDiffuse * fragColor;
}
