#version 460
layout(location = 0) out vec4 fragColor;
in vec3 ourColor;

void main()
{
   fragColor = vec4(ourColor, 1);
}