#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform bool isVoid;
uniform bool shouldClip;
uniform bool negativeBPM;

uniform vec4 tintLow;
uniform vec4 tintHigh;

out vec4 finalColor;

void main()
{
  float sign = negativeBPM ? -1.0 : 1.0;
  if (shouldClip && isVoid && (sign * fragPosition.z > 0.0)) discard;   // cut everything past the current playhead position

  // convert to arc y coord
  float h = clamp((fragPosition.y - 1.0) / 4.5, 0.0, 1.0);
  vec4 tint = mix(tintLow, tintHigh, h);

  vec4 texelColor = texture(texture0, fragTexCoord);
  finalColor = texelColor * tint * fragColor;
}
