#version 330

in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main()
{
    finalColor = vec4(texture(texture0, fragTexCoord).rgb, 1.0);
}
