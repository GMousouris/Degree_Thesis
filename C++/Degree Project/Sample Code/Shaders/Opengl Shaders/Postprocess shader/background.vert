#version 330 core

layout(location = 0) in vec4 attrPosition;
layout(location = 8) in vec2 attrTexCoord0;

out vec2 varTexCoord0;

void main(void)
{
   gl_Position  = attrPosition;
   varTexCoord0 = attrTexCoord0;
}