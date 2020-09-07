#version 330
layout(location = 0, index = 0) out vec4 outColor;

uniform sampler2D samplerHDR;
uniform sampler2D Depth_Sampler;
in vec2 varTexCoord0;

uniform vec3  colorBalance;
uniform float invWhitePoint;
uniform float burnHighlights;
uniform float saturation;
uniform float crushBlacks;
uniform float invGamma;
uniform int   apply_postproc_effects;
uniform int   multisample;
uniform int   uniform_discard_alpha;
uniform int   uniform_visualize_depth;
uniform int   uniform_write_depth;
uniform int   uniform_invert_pixels;

void main(void)
{
    vec3 hdrColor;
	vec3 ldrColor;
	float alpha;

	

	if ( uniform_write_depth == 1 )
	{
	    float depth  = texture(Depth_Sampler, varTexCoord0).r;
		gl_FragDepth = depth;
	}

	if ( uniform_visualize_depth == 1 )
	{
	    float depth = texture(samplerHDR, varTexCoord0).r;
		float n = 0.1; // camera z near
        float f = 1500.0f;//9999999999.f;
        float z = depth;
        float l_d = (2.0 * n) / (f + n - z * (f - n));	

		ldrColor = vec3(l_d);
		alpha    = 1.0f;
	}
	else
	{

	    hdrColor = texture(samplerHDR, varTexCoord0).rgb;
	    if (apply_postproc_effects == 1)
	    {
	    	alpha = 1.0f;
	    	ldrColor = invWhitePoint * colorBalance *  hdrColor;
	    	ldrColor *= (ldrColor * burnHighlights + 1.0) / (ldrColor + 1.0);
	    	float luminance = dot(ldrColor, vec3(0.3, 0.59, 0.11));
	    	ldrColor = max(mix(vec3(luminance), ldrColor, saturation), 0.0);
	    	luminance = dot(ldrColor, vec3(0.3, 0.59, 0.11));
	    	if (luminance < 1.0)
	    	{
	    		ldrColor = max(mix(pow(ldrColor, vec3(crushBlacks)), ldrColor, sqrt(luminance)), 0.0);
	    	}
	    	ldrColor = pow(ldrColor, vec3(invGamma));
	    }
	    else
	    {
	    	alpha = texture(samplerHDR, varTexCoord0).a;
	    	ldrColor = hdrColor;
	    }
    
    
	    if (uniform_discard_alpha == 1)
	    {
	    	if (alpha < 0.1f)
	    		discard;
	    }
    }

	outColor = vec4(ldrColor, alpha);
}