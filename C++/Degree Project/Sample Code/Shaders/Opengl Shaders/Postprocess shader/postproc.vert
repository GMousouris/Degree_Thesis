#version 330 core

layout(location = 0) in vec4 attrPosition;
layout(location = 8) in vec2 attrTexCoord0;

uniform int uniform_invert_pixels;

out vec2 varTexCoord0;

void main(void)
{
   gl_Position  = attrPosition;
   if ( uniform_invert_pixels == 1 )
   {
       varTexCoord0 = vec2( attrTexCoord0.x, 1.0f - attrTexCoord0.y );
   }
   else
   {
       varTexCoord0 = attrTexCoord0;
   }
}