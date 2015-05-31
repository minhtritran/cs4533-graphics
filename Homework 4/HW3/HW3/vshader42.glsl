/***************************
 * File: vshader42.glsl:
 *   A simple vertex shader.
 *
 * - Vertex attributes (positions & colors) for all vertices are sent
 *   to the GPU via a vertex buffer object created in the OpenGL program.
 *
 * - This vertex shader uses the Model-View and Projection matrices passed
 *   on from the OpenGL program as uniform variables of type mat4.
 ***************************/

 #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

#define M_PI 3.1415926535897932384626433832795

in  vec4 vPosition;
in	vec3 vNormal;
in  vec4 vColor;
in  vec2 vTexCoord;
in  vec4 vVelocity;
out vec4 color;
out vec2 texCoord;
out vec2 latticeTexCoord;
out float z;
out float discard_fireworks_particle;

uniform vec4 global_light_ambient;

uniform vec4 directional_light_ambient;
uniform vec4 directional_light_diffuse;
uniform vec4 directional_light_specular;
uniform vec4 directional_light_direction;

uniform vec4 point_light_ambient;
uniform vec4 point_light_diffuse;
uniform vec4 point_light_specular;
uniform vec4 point_light_position_eyeFrame;
uniform float point_const_att;
uniform float point_linear_att;
uniform float point_quad_att;

uniform vec4 spotlight_destination_position_eyeFrame;
uniform float spotlight_exponent;
uniform float spotlight_cutoff_angle;

uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform float material_shininess;

uniform float lighting_flag;
uniform float shading_flag;
uniform float light_source_flag;
uniform float vertical_slanted_flag;
uniform float object_eye_frame_flag;
uniform float upright_tilted_flag;
uniform float lattice_flag;
uniform float is_sphere_flag;
uniform float is_sphere_shadow_flag;
uniform float is_floor_flag;
uniform float is_fireworks_flag;
uniform float texture_sphere_flag;

uniform float elapsed_time;

uniform mat4 model_view;
uniform mat4 projection;

uniform mat3 Normal_Matrix;

void main() 
{
	
	if (is_fireworks_flag == 1) {
		vec3 pos = (model_view * vPosition).xyz;
		gl_Position = projection * vec4(
			pos.x + (0.0001 * vVelocity.x * elapsed_time),
			pos.y + (0.0001 * vVelocity.y * elapsed_time) + (0.5 * -0.00000049 * elapsed_time * elapsed_time),
			pos.z + (0.0001 * vVelocity.z * elapsed_time),
			1.0);
		color = vColor;
		if (gl_Position.y < 0.1)
			discard_fireworks_particle = 1;
		else
			discard_fireworks_particle = 0;
	}
	else {
		gl_Position = projection * model_view * vPosition;
		z = gl_Position.z;

		vec3 pos = (model_view * vPosition).xyz;
		if (lighting_flag == 0) {
			color = vColor;
		}
		else {
			vec3 L;
			vec3 E;
			vec3 H;
			vec3 N;

			// Transform vertex position into eye coordinates
			
			if (shading_flag == 1 && is_sphere_flag == 1) {
				N = normalize( model_view*vec4(vPosition.xyz, 0.0) ).xyz;
			}
			else {
				N = normalize( model_view*vec4(vNormal, 0.0) ).xyz;
				//N = normalize(Normal_Matrix * vNormal);
			}
			//GLOBAL AMBIENT LIGHT
			vec4 global_ambient = global_light_ambient * material_ambient;

			//DIRECTIONAL LIGHT SOURCE
			L = normalize( -directional_light_direction.xyz );
			E = normalize( -pos );
			H = normalize( L + E );

			float directional_attenuation = 1.0; 

			// Compute terms in the illumination equation
			vec4 directional_ambient = directional_light_ambient * material_ambient;

			float d = max( dot(L, N), 0.0 );
			vec4  directional_diffuse = d * directional_light_diffuse * material_diffuse;

			float s = pow( max(dot(N, H), 0.0), material_shininess );
			vec4  directional_specular = s * directional_light_specular * material_specular;
    
			if( dot(L, N) < 0.0 ) {
				directional_specular = vec4(0.0, 0.0, 0.0, 1.0);
			}

			//POINT LIGHT SOURCE
			// Transform vertex  position into eye coordinates
			L = normalize( point_light_position_eyeFrame.xyz - pos );
			E = normalize( -pos );
			H = normalize( L + E );

			/*--- To Do: Compute attenuation ---*/
			vec3 dist_vector = point_light_position_eyeFrame.xyz - pos.xyz;
			float dist_magnitude = pow(dist_vector.x * dist_vector.x + dist_vector.y * dist_vector.y + dist_vector.z * dist_vector.z, 0.5);
			float dist_attenuation = 1 / (point_const_att + point_linear_att * dist_magnitude + point_quad_att * dist_magnitude * dist_magnitude);

			vec3 spotlight_direction = normalize(spotlight_destination_position_eyeFrame.xyz - point_light_position_eyeFrame.xyz);
			float spotlight_attenuation = 0.0;
			if (dot(-L, spotlight_direction) >= cos(spotlight_cutoff_angle * M_PI / 180))
				spotlight_attenuation = pow(dot(-L, spotlight_direction), spotlight_exponent);

			float point_attenuation = dist_attenuation;
			if (light_source_flag == 0)
				point_attenuation = point_attenuation * spotlight_attenuation;


			// Compute terms in the illumination equation
			vec4 point_ambient = point_light_ambient * material_ambient;

			d = max( dot(L, N), 0.0 );
			vec4  point_diffuse = d * point_light_diffuse * material_diffuse;

			s = pow( max(dot(N, H), 0.0), material_shininess );
			vec4  point_specular = s * point_light_specular * material_specular;
    
			if( dot(L, N) < 0.0 ) {
				point_specular = vec4(0.0, 0.0, 0.0, 1.0);
			}

			/*--- attenuation below must be computed properly ---*/
			color = global_ambient + directional_attenuation * (directional_ambient + directional_diffuse + directional_specular) + point_attenuation * (point_ambient + point_diffuse + point_specular);
		}
		
		if (is_floor_flag) {
			texCoord = vTexCoord;
		}
		else if (is_sphere_flag) {
			if (texture_sphere_flag == 1) {
				if (vertical_slanted_flag == 0 && object_eye_frame_flag == 0)
					texCoord = vec2(2.5 * vPosition.x, 0.0);
				else if (vertical_slanted_flag == 1 && object_eye_frame_flag == 0)
					texCoord = vec2(1.5 * (vPosition.x + vPosition.y + vPosition.z), 0.0);
				else if (vertical_slanted_flag == 0 && object_eye_frame_flag == 1)
					texCoord = vec2(2.5 * pos.x, 0.0);
				else if (vertical_slanted_flag == 1 && object_eye_frame_flag == 1)
					texCoord = vec2(1.5 * (pos.x + pos.y + pos.z), 0.0);
			}
			else if (texture_sphere_flag == 2) {
				if (vertical_slanted_flag == 0 && object_eye_frame_flag == 0)
					texCoord = vec2(0.5 * (vPosition.x + 1), 0.5 * (vPosition.y + 1));
				else if (vertical_slanted_flag == 1 && object_eye_frame_flag == 0)
					texCoord = vec2(0.3 * (vPosition.x + vPosition.y + vPosition.z), 0.3 * (vPosition.x - vPosition.y + vPosition.z));
				else if (vertical_slanted_flag == 0 && object_eye_frame_flag == 1)
					texCoord = vec2(0.5 * (pos.x + 1), 0.5 * (pos.y + 1));
				else if (vertical_slanted_flag == 1 && object_eye_frame_flag == 1)
					texCoord = vec2(0.3 * (pos.x + pos.y + pos.z), 0.3 * (pos.x - pos.y + pos.z));
			}
		}
	
		if ((is_sphere_flag == 1 || is_sphere_shadow_flag == 1) && lattice_flag == 1) {
			if (upright_tilted_flag == 0)
				latticeTexCoord = vec2(0.5 * (vPosition.x + 1), 0.5 * (vPosition.y + 1));
			else if (upright_tilted_flag == 1)
				latticeTexCoord = vec2(0.3 * (vPosition.x + vPosition.y + vPosition.z), 0.3 * (vPosition.x - vPosition.y + vPosition.z));
		}
	}
} 
