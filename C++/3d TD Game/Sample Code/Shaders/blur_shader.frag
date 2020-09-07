#version 330 core

in vec2 f_texcoord;

uniform sampler2D uniform_texture;
uniform vec2 uniform_scale_u;

out vec4 out_color;

void main()
{

  vec4 color = texture2D(uniform_texture, f_texcoord , 0);
  color = vec4(0);

  //vec2 scale_u = uniform_scale_u;
  vec2 scale_u = uniform_scale_u*1.f;

  color += texture2D( uniform_texture, f_texcoord.st + vec2( -3.0*scale_u.x, -3.0*scale_u.y ) ) * 0.015625;
  color += texture2D( uniform_texture, f_texcoord.st + vec2( -2.0*scale_u.x, -2.0*scale_u.y ) )*0.09375;
  color += texture2D( uniform_texture, f_texcoord.st + vec2( -1.0*scale_u.x, -1.0*scale_u.y ) )*0.234375;
  color += texture2D( uniform_texture, f_texcoord.st + vec2( 0.0 , 0.0) )*0.3125;
  color += texture2D( uniform_texture, f_texcoord.st + vec2( 1.0*scale_u.x,  1.0*scale_u.y ) )*0.234375;
  color += texture2D( uniform_texture, f_texcoord.st + vec2( 2.0*scale_u.x,  2.0*scale_u.y ) )*0.09375;
  color += texture2D( uniform_texture, f_texcoord.st + vec2( 3.0*scale_u.x, -3.0*scale_u.y ) ) * 0.015625;

  out_color = color;

};