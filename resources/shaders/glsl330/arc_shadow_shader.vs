#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;

uniform float worldX1;
uniform float worldX2;
uniform float lengthDelta;
uniform float scale;

out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragPosition;   // world-space position, passer to clip against

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    vec3 final_vertex_position = vec3(vertexPosition);
    final_vertex_position.x *= scale;
    bool is_front = final_vertex_position.z < 0.5;
    if (is_front)
    {
      final_vertex_position.x += worldX1;
    } else
    {
      final_vertex_position.x += worldX2;
      final_vertex_position.z *= lengthDelta;
    }
    fragPosition = vec3(matModel * vec4(final_vertex_position, 1.0)); // transform to world space

    gl_Position = mvp * vec4(final_vertex_position, 1.0);
}
