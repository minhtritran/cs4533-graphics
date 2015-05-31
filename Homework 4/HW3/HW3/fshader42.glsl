/*****************************
 * File: fshader42.glsl
 *       A simple fragment shader
 *****************************/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 color;
in  vec2 texCoord;
in  vec2 latticeTexCoord;
in float z;
in float discard_fireworks_particle;
out vec4 fColor;

uniform sampler2D texture_2D;
uniform float is_sphere_flag;
uniform float is_sphere_shadow_flag;
uniform float is_floor_flag;
uniform float texture_ground_flag;
uniform float texture_sphere_flag;
uniform float lattice_flag;
uniform float fog_linear_start;
uniform float fog_linear_end;
uniform float fog_exponential_density;
uniform vec4 fog_color;
uniform float fog_flag;

uniform float is_fireworks_flag;

void main() 
{
	if (discard_fireworks_particle == 1) {
		discard;
	}
	if (is_fireworks_flag == 1) {
		fColor = color;
	}
	else {
		if ((is_sphere_flag == 1 || is_sphere_shadow_flag == 1) && lattice_flag == 1) {
			if (fract(4 * latticeTexCoord[0]) < 0.35 && fract(4 * latticeTexCoord[1]) < 0.35)
				discard;
		}
	
		if (is_floor_flag == 1 && texture_ground_flag == 1) {
			fColor = color * texture(texture_2D, texCoord);
		}
		else if (is_sphere_flag == 1 && texture_sphere_flag != 0) {
			if (texture_sphere_flag == 2 && texture(texture_2D, texCoord).r == 0.0)
				fColor = color * vec4(0.9, 0.1, 0.1, 1.0);
			else
				fColor = color * texture(texture_2D, texCoord);
		}
		else {
			fColor = color;
		}
	
		if (fog_flag != 0) {
			float fog_factor = 0.0;
			//linear fog
			if (fog_flag == 1) {
				fog_factor = (fog_linear_end - z) / (fog_linear_end - fog_linear_start);
			}
			//exponential fog
			else if (fog_flag == 2) {
				fog_factor = exp(-(fog_exponential_density * z));
			}
			//exponential squared fog
			else if (fog_flag == 3) {
				fog_factor = exp(-pow((fog_exponential_density * z), 2));
			}
			fog_factor = clamp(fog_factor, 0, 1);
			fColor = mix(fog_color, fColor, fog_factor);
		}
	}
} 

