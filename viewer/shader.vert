#version 460
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 ccolor;

uniform mat4 theMatrix;

out vec3 ourColor;

void main()
{
  gl_Position = theMatrix * vec4(position.x, position.y, position.z, 1.0);
  ourColor = ccolor;
}