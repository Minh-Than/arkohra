#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float enwidenlanesValue;

out vec4 finalColor;

void main()
{
  vec4 tint = fragColor;
  vec2 uv = fragTexCoord;
  if (uv.x < 0.03515625 || uv.x > 0.96484375) tint.a *= (1.0 - enwidenlanesValue);
  vec4 texelColor = texture(texture0, fragTexCoord);
  finalColor = texelColor * tint * colDiffuse;
}
