/************************************************************
 * Handout: rotate-cube-new.cpp (A Sample Code for Shader-Based OpenGL ---
                                 for OpenGL version 3.1 and later)
 * Originally from Ed Angel's textbook "Interactive Computer Graphics" 6th Ed
              sample code "example3.cpp" of Chapter 4.
 * Moodified by Yi-Jen Chiang to include the use of a general rotation function
   Rotate(angle, x, y, z), where the vector (x, y, z) can have length != 1.0,
   and also to include the use of the function NormalMatrix(mv) to return the
   normal matrix (mat3) of a given model-view matrix mv (mat4).

   (The functions Rotate() and NormalMatrix() are added to the file "mat-yjc-new.h"
   by Yi-Jen Chiang, where a new and correct transpose function "transpose1()" and
   other related functions such as inverse(m) for the inverse of 3x3 matrix m are
   also added; see the file "mat-yjc-new.h".)

 * Extensively modified by Yi-Jen Chiang for the program structure and user
   interactions. See the function keyboard() for the keyboard actions.
   Also extensively re-structured by Yi-Jen Chiang to create and use the new
   function drawObj() so that it is easier to draw multiple objects. Now a floor
   and a rotating cube are drawn.

** Perspective view of a color cube using LookAt() and Perspective()

** Colors are assigned to each vertex and then the rasterizer interpolates
   those colors across the triangles.
**************************************************************/

#include "Angel-yjc.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

/* global definitions for constants and global image arrays */
#define checkImageWidth  64
#define checkImageHeight 64
static GLubyte checkImage[checkImageHeight][checkImageWidth][4];

static GLuint checkTexture;
static GLuint stripeTexture;

#define	stripeImageWidth 32
GLubyte stripeImage[4 * stripeImageWidth];

GLuint program;       /* shader program object id */
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint sphere_buffer;
GLuint sphere_shadow_buffer;
GLuint axes_buffer;
GLuint fireworks_buffer;

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 18.0;

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'

int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'
int sphereFlag = 1;

int shadowFlag = 1;
int lightingFlag = 1;
int shadingFlag = 1;
int lightSourceFlag = 1;
int fogFlag = 0;
int shadowBlendingFlag = 1;
int textureGroundFlag = 1;
int verticalSlantedFlag = 0;
int objectEyeFrameFlag = 0;
int textureSphereFlag = 1;
int uprightTiltedFlag = 0;
int latticeFlag = 0;
int fireworksFlag = 1;

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 floor_points[floor_NumVertices]; // positions for all vertices
vec3 floor_normals[floor_NumVertices];
color4 floor_colors[floor_NumVertices]; // colors for all vertices
vec2 floor_texCoord[floor_NumVertices];

int sphere_NumVertices;
point4 sphere_points[3200];
vec3 sphere_normals[3200];
color4 sphere_colors[3200];
color4 sphere_shadow_colors[3200];

const int axes_NumVertices = 6;  //(3 axis)*(1 lines/axis)*(2 vertices/line)
point4 axes_points[axes_NumVertices];
color4 axes_colors[axes_NumVertices];

const int fireworks_NumParticles = 300;
point4 fireworks_points[fireworks_NumParticles];
color4 fireworks_colors[fireworks_NumParticles];
vec4 fireworks_velocities[fireworks_NumParticles];

float elapsed_time = 0.0;
float sub_time = 0.0f;
int max_time = 5000;

int pathState = 0;

vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
vec4 pointA = vec4(3.0, 1.0, 5.0, 1.0);
vec4 pointB = vec4(-2.0, 1.0, -2.5, 1.0);
vec4 pointC = vec4(2.0, 1.0, -4.0, 1.0);

vec4 translate = pointA;

vec4 vecOY = vec4(0.0, 1.0, 0.0, 0.0);
double rotateX, rotateY, rotateZ;

mat4 accum_rotation = mat4();

//vec4 light_source = vec4(-14.0, 12.0, -3.0, 1.0);

//global ambient light
color4 global_light_ambient(1.0, 1.0, 1.0, 1.0);

//directional light source
color4 directional_light_ambient(0.0, 0.0, 0.0, 1.0);
color4 directional_light_diffuse(0.8, 0.8, 0.8, 1.0);
color4 directional_light_specular(0.2, 0.2, 0.2, 1.0);
vec4 directional_light_direction(0.1, 0.0, -1.0, 0.0);

//point source
color4 point_light_ambient(0.0, 0.0, 0.0, 1.0);
color4 point_light_diffuse(1.0, 1.0, 1.0, 1.0);
color4 point_light_specular(1.0, 1.0, 1.0, 1.0);
vec4 point_light_position(-14.0, 12.0, -3.0, 1.0);
float point_const_att = 2.0;
float point_linear_att = 0.01;
float point_quad_att = 0.001;

//spotlight
vec4 spotlight_destination_position(-6.0, 0.0, -4.5, 1.0);
float spotlight_exponent = 15.0;
float splotlight_cutoff_angle = 20.0;

//background material
color4 ground_material_ambient(0.2, 0.2, 0.2, 1.0);
color4 ground_material_diffuse(0.0, 1.0, 0.0, 1.0);
color4 ground_material_specular(0.0, 0.0, 0.0, 1.0);
float  ground_material_shininess = 0.0;

//sphere material
color4 sphere_material_ambient(0.2, 0.2, 0.2, 1.0);
color4 sphere_material_diffuse(1.0, 0.84, 0.0, 1.0);
color4 sphere_material_specular(1.0, 0.84, 0.0, 1.0);
float  sphere_material_shininess = 125.0;

//fog values
float fog_linear_start = 0.0;
float fog_linear_end = 18.0;
float fog_exponential_density = 0.09;
color4 fog_color(0.7, 0.7, 0.7, 0.5);

//----------------------------------------------------------------------------
int Index = 0; // YJC: This must be a global variable since quad() is called
               //      multiple times and Index should then go up to 36 for
               //      the 36 vertices and colors

// quad(): generate two triangles for each face and assign colors to the vertices

//-------------------------------
// generate 2 triangles: 6 vertices and 6 colors
void floor()
{
	point4 p1 = point4(5.0, 0.0, 8.0, 1.0);
	point4 p2 = point4(5.0, 0.0, -4.0, 1.0);
	point4 p3 = point4(-5.0, 0.0, -4.0, 1.0);
	point4 p4 = point4(-5.0, 0.0, 8.0, 1.0);

	color4 c = color4(0.0, 1.0, 0.0, 1.0);
	vec3 normal = vec3(0.0, 1.0, 0.0);

	floor_colors[0] = c; floor_points[0] = p1; floor_normals[0] = normal;
	floor_colors[1] = c; floor_points[1] = p2; floor_normals[1] = normal;
	floor_colors[2] = c; floor_points[2] = p3; floor_normals[2] = normal;

	floor_colors[3] = c; floor_points[3] = p3; floor_normals[3] = normal;
	floor_colors[4] = c; floor_points[4] = p4; floor_normals[4] = normal;
	floor_colors[5] = c; floor_points[5] = p1; floor_normals[5] = normal;

	floor_texCoord[0] = vec2(0.0, 0.0);
	floor_texCoord[1] = vec2(0.0, 1.5);
	floor_texCoord[2] = vec2(1.25, 1.5);

	floor_texCoord[3] = vec2(1.25, 1.5);
	floor_texCoord[4] = vec2(1.25, 0.0);
	floor_texCoord[5] = vec2(0.0, 0.0);
}

//----------------------------------------------------------------------------

void axes() {
	point4 origin = point4(0.0, 0.0, 0.0, 1.0);
	point4 x_axis_point = point4(10.0, 0.0, 0.0, 1.0);
	point4 y_axis_point = point4(0.0, 10.0, 0.0, 1.0);
	point4 z_axis_point = point4(0.0, 0.0, 10.0, 1.0);
	color4 red = color4(1.0, 0.0, 0.0, 1.0);
	color4 magenta = color4(1.0, 0.0, 1.0, 1.0);
	color4 blue = color4(0.0, 0.0, 1.0, 1.0);

	axes_colors[0] = red; axes_points[0] = origin;
	axes_colors[1] = red; axes_points[1] = x_axis_point;

	axes_colors[2] = magenta; axes_points[2] = origin;
	axes_colors[3] = magenta; axes_points[3] = y_axis_point;

	axes_colors[4] = blue; axes_points[4] = origin;
	axes_colors[5] = blue; axes_points[5] = z_axis_point;
}

void fireworks() {
	for (int i = 0; i < fireworks_NumParticles; i++) {
		fireworks_points[i] = point4(0.0, 0.1, 0.0, 1.0);
		fireworks_colors[i] = color4(
			(rand() % 256) / 256.0,
			(rand() % 256) / 256.0,
			(rand() % 256) / 256.0,
			1.0);
		fireworks_velocities[i] = vec4(
			10.0*2.0*((rand() % 256) / 256.0 - 0.5),
			10.0*1.2*2.0*((rand() % 256) / 256.0),
			10.0*2.0*((rand() % 256) / 256.0 - 0.5),
			0.0);
	}
}

void readFile() {
	cout << "Please type in a filename." << endl;
	string filename;
	cin >> filename;

	ifstream ifs(filename);
	int numPolygons;
	int numVertices;
	int index = 0;
	ifs >> numPolygons;
	for (int i = 0; i < numPolygons; i++) {
		ifs >> numVertices;
		vec3 vertices[3];

		for (int j = 0; j < numVertices; j++) {
			ifs >> vertices[j][0] >> vertices[j][1] >> vertices[j][2];
		}

		vec4 u = vertices[1] - vertices[0];
		vec4 v = vertices[2] - vertices[1];
		vec3 normal = normalize(cross(u, v));

		for (int j = 0; j < numVertices; j++) {
			sphere_points[index] = point4(vertices[j][0], vertices[j][1], vertices[j][2], 1.0);
			sphere_normals[index] = normal;
			sphere_colors[index] = color4(1.0, 0.84, 0.0, 1.0);
			sphere_shadow_colors[index] = color4(0.25, 0.25, 0.25, 0.65);

			index++;
		}

		
	}

	sphere_NumVertices = numPolygons * numVertices;

}



/*************************************************************
void image_set_up(void):
generate checkerboard and stripe images.

* Inside init(), call this function and set up texture objects
for texture mapping.
(init() is called from main() before calling glutMainLoop().)
***************************************************************/
void image_set_up(void)
{
	int i, j, c;

	/* --- Generate checkerboard image to the image array ---*/
	for (i = 0; i < checkImageHeight; i++)
		for (j = 0; j < checkImageWidth; j++)
		{
		c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

		if (c == 1) /* white */
		{
			c = 255;
			checkImage[i][j][0] = (GLubyte)c;
			checkImage[i][j][1] = (GLubyte)c;
			checkImage[i][j][2] = (GLubyte)c;
		}
		else  /* green */
		{
			checkImage[i][j][0] = (GLubyte)0;
			checkImage[i][j][1] = (GLubyte)150;
			checkImage[i][j][2] = (GLubyte)0;
		}

		checkImage[i][j][3] = (GLubyte)255;
		}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*--- Generate 1D stripe image to array stripeImage[] ---*/
	for (j = 0; j < stripeImageWidth; j++) {
		/* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
		When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
		*/
		stripeImage[4 * j] = (GLubyte)255;
		stripeImage[4 * j + 1] = (GLubyte)((j>4) ? 255 : 0);
		stripeImage[4 * j + 2] = (GLubyte)0;
		stripeImage[4 * j + 3] = (GLubyte)255;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	/*----------- End 1D stripe image ----------------*/

	/*--- texture mapping set-up is to be done in
	init() (set up texture objects),
	display() (activate the texture object to be used, etc.)
	and in shaders.
	---*/

} /* end function */


// OpenGL initialization
void init()
{
	readFile();

#if 0 //YJC: The following is not needed
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
#endif

    floor();     
 // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(vec3) * floor_NumVertices + sizeof(vec2) * floor_NumVertices,
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
                    floor_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors), sizeof(vec3) * floor_NumVertices, floor_normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(vec3) * floor_NumVertices, sizeof(vec2) * floor_NumVertices, floor_texCoord);

 // Sphere
	glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices + sizeof(vec3) * sphere_NumVertices,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices, sphere_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices, sizeof(color4) * sphere_NumVertices,
		sphere_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices, sizeof(vec3) * sphere_NumVertices, sphere_normals);

 // Sphere shadow
	glGenBuffers(1, &sphere_shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_shadow_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices + sizeof(vec3) * sphere_NumVertices,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices, sphere_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices, sizeof(color4) * sphere_NumVertices,
		sphere_shadow_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices, sizeof(vec3) * sphere_NumVertices, sphere_normals);

 // Axes
	axes();
	glGenBuffers(1, &axes_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axes_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * axes_NumVertices + sizeof(color4) * axes_NumVertices + sizeof(vec3) * sphere_NumVertices,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * axes_NumVertices, axes_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * axes_NumVertices, sizeof(color4) * axes_NumVertices,
		axes_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * axes_NumVertices + sizeof(color4) * axes_NumVertices, sizeof(vec3) * sphere_NumVertices, sphere_normals);

 // Fireworks
	fireworks();
	glGenBuffers(1, &fireworks_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, fireworks_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * fireworks_NumParticles + sizeof(color4) * fireworks_NumParticles + sizeof(vec4) * fireworks_NumParticles,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * fireworks_NumParticles, fireworks_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * fireworks_NumParticles, sizeof(color4) * fireworks_NumParticles,
		fireworks_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * fireworks_NumParticles + sizeof(color4) * fireworks_NumParticles, sizeof(vec4) * fireworks_NumParticles, fireworks_velocities);
	
 // Load shaders and create a shader program (to be used in display())
    program = InitShader("vshader42.glsl", "fshader42.glsl");
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.529, 0.807, 0.92, 0.0 ); 
    glLineWidth(2.0);
	glPointSize(3.0);

	image_set_up();

	/*--- Create and Initialize a texture object ---*/
	glGenTextures(1, &checkTexture);      // Generate texture obj name(s)

	glActiveTexture(GL_TEXTURE0);  // Set the active texture unit to be 0 
	glBindTexture(GL_TEXTURE_2D, checkTexture); // Bind the texture to this texture unit

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);


	glGenTextures(1, &stripeTexture);

	glActiveTexture(GL_TEXTURE1);  // Set the active texture unit to be 0 
	glBindTexture(GL_TEXTURE_2D, stripeTexture); // Bind the texture to this texture unit

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, stripeImageWidth, stripeImageWidth,
		0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);

	/** Note: If using multiple textures, repeat the above process starting from
	glActiveTexture(), but each time use a *different texture unit*,
	so that each texture is bound to a *different texture unit*.    **/

}
//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices, GLuint drawType)
{
	if (buffer == sphere_shadow_buffer && shadowBlendingFlag == 1) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	GLuint vColor = glGetAttribLocation(program, "vColor");
	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
	GLuint vVelocity = glGetAttribLocation(program, "vVelocity");
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    
	if (buffer == fireworks_buffer) {
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(0));

		glEnableVertexAttribArray(vColor);
		glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point4) * num_vertices));

		glEnableVertexAttribArray(vVelocity);
		glVertexAttribPointer(vVelocity, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point4) * num_vertices + sizeof(color4) * num_vertices));

	}
	else {
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(0));

		glEnableVertexAttribArray(vColor);
		glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point4) * num_vertices));
		// the offset is the (total) size of the previous vertex attribute array(s)

		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point4) * num_vertices + sizeof(color4) * num_vertices));

		glEnableVertexAttribArray(vTexCoord);
		glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point4) * num_vertices + sizeof(color4) * num_vertices + sizeof(vec3) * num_vertices));
	}
    
	if (buffer == floor_buffer) {
		glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
	}
	else if (buffer == sphere_buffer) {
		if (textureSphereFlag == 1)
			glUniform1i(glGetUniformLocation(program, "texture_2D"), 1);
		else if (textureSphereFlag == 2)
			glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
	}
	if (buffer == sphere_buffer) {
		glUniform4fv(glGetUniformLocation(program, "material_ambient"), 1, sphere_material_ambient);
		glUniform4fv(glGetUniformLocation(program, "material_diffuse"), 1, sphere_material_diffuse);
		glUniform4fv(glGetUniformLocation(program, "material_specular"), 1, sphere_material_specular);
		glUniform1f(glGetUniformLocation(program, "material_shininess"), sphere_material_shininess);
	}
	else {
		glUniform4fv(glGetUniformLocation(program, "material_ambient"), 1, ground_material_ambient);
		glUniform4fv(glGetUniformLocation(program, "material_diffuse"), 1, ground_material_diffuse);
		glUniform4fv(glGetUniformLocation(program, "material_specular"), 1, ground_material_specular);
		glUniform1f(glGetUniformLocation(program, "material_shininess"), ground_material_shininess);
	}

	if (buffer == axes_buffer || buffer == sphere_shadow_buffer || (buffer == sphere_buffer && sphereFlag == 0))
		glUniform1f(glGetUniformLocation(program, "lighting_flag"), 0);
	else
		glUniform1f(glGetUniformLocation(program, "lighting_flag"), lightingFlag);

	if (buffer == sphere_buffer) {
		glUniform1f(glGetUniformLocation(program, "is_sphere_flag"), 1);
	}
	else {
		glUniform1f(glGetUniformLocation(program, "is_sphere_flag"), 0);
	}

	if (buffer == sphere_shadow_buffer) {
		glUniform1f(glGetUniformLocation(program, "is_sphere_shadow_flag"), 1);
	}
	else {
		glUniform1f(glGetUniformLocation(program, "is_sphere_shadow_flag"), 0);
	}

	if (buffer == floor_buffer) {
		glUniform1f(glGetUniformLocation(program, "is_floor_flag"), 1);
	}
	else {
		glUniform1f(glGetUniformLocation(program, "is_floor_flag"), 0);
	}

	glUniform1f(glGetUniformLocation(program, "texture_ground_flag"), textureGroundFlag);

	if (buffer == sphere_buffer && sphereFlag == 1)
		glUniform1f(glGetUniformLocation(program, "texture_sphere_flag"), textureSphereFlag);
	else
		glUniform1f(glGetUniformLocation(program, "texture_sphere_flag"), 0);

	if (buffer == fireworks_buffer)
		glUniform1f(glGetUniformLocation(program, "is_fireworks_flag"), 1);
	else
		glUniform1f(glGetUniformLocation(program, "is_fireworks_flag"), 0);
    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(drawType, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
	glDisableVertexAttribArray(vNormal);
	glDisableVertexAttribArray(vTexCoord);
	if (buffer == sphere_shadow_buffer && shadowBlendingFlag == 1) {
		glDisable(GL_BLEND);
	}
}
//----------------------------------------------------------------------------
void display( void )
{
	GLuint  model_view;  // model-view matrix uniform shader variable location
	GLuint  projection;  // projection matrix uniform shader variable location

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram(program); // Use the shader program

    model_view = glGetUniformLocation(program, "model_view" );
    projection = glGetUniformLocation(program, "projection" );

    /*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

    /*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4    at(0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);

	mat4 mv;
    //mat4  mv = LookAt(eye, at, up);

	glUniform4fv(glGetUniformLocation(program, "global_light_ambient"), 1, global_light_ambient);
	glUniform4fv(glGetUniformLocation(program, "directional_light_ambient"), 1, directional_light_ambient);
	glUniform4fv(glGetUniformLocation(program, "directional_light_diffuse"), 1, directional_light_diffuse);
	glUniform4fv(glGetUniformLocation(program, "directional_light_specular"), 1, directional_light_specular);
	glUniform4fv(glGetUniformLocation(program, "directional_light_direction"), 1, directional_light_direction);
	glUniform4fv(glGetUniformLocation(program, "point_light_ambient"), 1, point_light_ambient);
	glUniform4fv(glGetUniformLocation(program, "point_light_diffuse"), 1, point_light_diffuse);
	glUniform4fv(glGetUniformLocation(program, "point_light_specular"), 1, point_light_specular);

	glUniform1f(glGetUniformLocation(program, "point_const_att"), point_const_att);
	glUniform1f(glGetUniformLocation(program, "point_linear_att"), point_linear_att);
	glUniform1f(glGetUniformLocation(program, "point_quad_att"), point_quad_att);

	glUniform1f(glGetUniformLocation(program, "spotlight_exponent"), spotlight_exponent);
	glUniform1f(glGetUniformLocation(program, "splotlight_cutoff_angle"), splotlight_cutoff_angle);

	glUniform1f(glGetUniformLocation(program, "shading_flag"), shadingFlag);
	glUniform1f(glGetUniformLocation(program, "light_source_flag"), lightSourceFlag);
	glUniform1f(glGetUniformLocation(program, "vertical_slanted_flag"), verticalSlantedFlag);
	glUniform1f(glGetUniformLocation(program, "object_eye_frame_flag"), objectEyeFrameFlag);
	glUniform1f(glGetUniformLocation(program, "upright_tilted_flag"), uprightTiltedFlag);
	glUniform1f(glGetUniformLocation(program, "lattice_flag"), latticeFlag);

	glUniform1f(glGetUniformLocation(program, "fog_linear_start"), fog_linear_start);
	glUniform1f(glGetUniformLocation(program, "fog_linear_end"), fog_linear_end);
	glUniform1f(glGetUniformLocation(program, "fog_exponential_density"), fog_exponential_density);
	glUniform4fv(glGetUniformLocation(program, "fog_color"), 1, fog_color);
	glUniform1f(glGetUniformLocation(program, "fog_flag"), fogFlag);

	glUniform1f(glGetUniformLocation(program, "elapsed_time"), elapsed_time);

	mv = LookAt(eye, at, up) * Translate(translate) * Rotate(angle, rotateX, rotateY, rotateZ) * accum_rotation;
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	if (sphereFlag == 1) // Filled sphere
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else              // Wireframe sphere
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawObj(sphere_buffer, sphere_NumVertices, GL_TRIANGLES);  // draw the sphere
	
	glDepthMask(GL_FALSE);

	mv = LookAt(eye, at, up);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	if (floorFlag == 1) // Filled floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else              // Wireframe floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawObj(floor_buffer, floor_NumVertices, GL_TRIANGLES);  // draw the floor

	if (shadowFlag == 1) {
		mat4 shadow = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, -1.0 / point_light_position.y, 0.0, 0.0));
		mat4 N = Translate(point_light_position.x, 0.0, point_light_position.z) * shadow * Translate(-point_light_position.x, -point_light_position.y, -point_light_position.z);
		mv = LookAt(eye, at, up) * N * Translate(translate) * Rotate(angle, rotateX, rotateY, rotateZ) * accum_rotation;
		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
		if (sphereFlag == 1) // Filled sphere
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe sphere
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawObj(sphere_shadow_buffer, sphere_NumVertices, GL_TRIANGLES);  // draw the sphere
	}

	glDepthMask(GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	mv = LookAt(eye, at, up);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	if (floorFlag == 1) // Filled floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else              // Wireframe floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawObj(floor_buffer, floor_NumVertices, GL_TRIANGLES);  // draw the floor

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	if (fireworksFlag == 1) {
		mv = LookAt(eye, at, up);
		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawObj(fireworks_buffer, fireworks_NumParticles, GL_POINTS);  // draw the floor
	}

	mv = LookAt(eye, at, up);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	drawObj(axes_buffer, axes_NumVertices, GL_LINES);  // draw the axes
	

	mat3 normal_matrix = NormalMatrix(mv, 0);
	glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);

	vec4 point_light_position_eyeFrame = mv * point_light_position;
	glUniform4fv(glGetUniformLocation(program, "point_light_position_eyeFrame"), 1, point_light_position_eyeFrame);

	vec4 spotlight_destination_position_eyeFrame= mv * spotlight_destination_position;
	glUniform4fv(glGetUniformLocation(program, "spotlight_destination_position_eyeFrame"), 1, spotlight_destination_position_eyeFrame);

    glutSwapBuffers();
}
//---------------------------------------------------------------------------
void idle (void)
{
	if (animationFlag)
		angle += 0.02;

	vec4 d = (angle / 360) * 2 * M_PI * 1.0;
	vec4 direction;
	vec3 rotateAxis;
	//From A to B
	if (pathState == 0) {
		direction = pointB - pointA;
		rotateAxis = cross(vecOY, direction);
		translate = (pointA - origin) + d * normalize(direction);
	}
	//From B to C
	else if (pathState == 1) {
		direction = pointC - pointB;
		rotateAxis = cross(vecOY, direction);
		translate = (pointB - origin) + d * normalize(direction);
	}
	//From C to A
	else if (pathState == 2) {
		direction = pointA - pointC;
		rotateAxis = cross(vecOY, direction);
		translate = (pointC - origin) + d * normalize(direction);
	}
	translate.w = 0.0;
	rotateX = rotateAxis.x;
	rotateY = rotateAxis.y;
	rotateZ = rotateAxis.z;


	if (translate.z < pointB.z && translate.x < pointB.x && direction.z < 0 && direction.x < 0) {
		accum_rotation = Rotate(angle, rotateX, rotateY, rotateZ) * accum_rotation;
		pathState = 1;
		angle = 0;
	}
	else if (translate.z < pointC.z && translate.x > pointC.x && direction.z < 0 && direction.x > 0) {
		accum_rotation = Rotate(angle, rotateX, rotateY, rotateZ) * accum_rotation;
		pathState = 2;
		angle = 0;
	}
	else if (translate.z > pointA.z && direction.z > 0) {
		accum_rotation = Rotate(angle, rotateX, rotateY, rotateZ) * accum_rotation;
		pathState = 0;
		angle = 0;
	}

	elapsed_time = glutGet(GLUT_ELAPSED_TIME) % max_time;

	glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;

	case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;
	case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;
	case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;

	case 'b': case 'B': // Toggle between animation and non-animation
	    animationFlag = 1 -  animationFlag;
        if (animationFlag == 1) glutIdleFunc(idle);
        else glutIdleFunc(NULL);
        break;
	   
	case 'f': case 'F': // Toggle between filled and wireframe floor
	    floorFlag = 1 - floorFlag; 
         break;

	case ' ':  // reset to initial viewer/eye position
	    eye = init_eye;
	    break;

	case 'v': case 'V':
		verticalSlantedFlag = 0;
		break;

	case 's': case 'S':
		verticalSlantedFlag = 1;
		break;

	case 'o': case 'O':
		objectEyeFrameFlag = 0;
		break;

	case 'e': case 'E':
		objectEyeFrameFlag = 1;
		break;

	case 'u': case 'U':
		uprightTiltedFlag = 0;
		break;

	case 't': case 'T':
		uprightTiltedFlag = 1;
		break;

	case 'l': case 'L':
		latticeFlag = 1 - latticeFlag;
		break;

    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		animationFlag = 1 - animationFlag;
	}
}
//----------------------------------------------------------------------------
void menu(int id) {
	switch (id) {
	case 1:
		exit(0);
		break;
	case 2:
		eye = init_eye;
		break;
	case 3:
		sphereFlag = 1 - sphereFlag;
		break;
	}
	animationFlag = 1;
	glutPostRedisplay();
}

void shadow_menu(int id) {
	switch (id) {
	case 1:
		shadowFlag = 0;
		break;
	case 2:
		shadowFlag = 1;
		break;

	}
	glutPostRedisplay();
}

void lighting_menu(int id) {
	switch (id) {
	case 1:
		lightingFlag = 0;
		break;
	case 2:
		lightingFlag = 1;
		break;

	}
	glutPostRedisplay();
}

void shading_menu(int id) {
	switch (id) {
	case 1:
		shadingFlag = 0;
		sphereFlag = 1;
		break;
	case 2:
		shadingFlag = 1;
		sphereFlag = 1;
		break;

	}
	glutPostRedisplay();
}

void light_source_menu(int id) {
	switch (id) {
	case 1:
		lightSourceFlag = 0;
		break;
	case 2:
		lightSourceFlag = 1;
		break;

	}
	glutPostRedisplay();
}

void fog_menu(int id) {
	switch (id) {
	case 1:
		fogFlag = 0;
		break;
	case 2:
		fogFlag = 1;
		break;
	case 3:
		fogFlag = 2;
		break;
	case 4:
		fogFlag = 3;
		break;
	}
	glutPostRedisplay();
}

void shadow_blending_menu(int id) {
	switch (id) {
	case 1:
		shadowBlendingFlag = 0;
		break;
	case 2:
		shadowBlendingFlag = 1;
		break;
	}
	glutPostRedisplay();
}

void texture_ground_menu(int id) {
	switch (id) {
	case 1:
		textureGroundFlag = 0;
		break;
	case 2:
		textureGroundFlag = 1;
		break;
	}
	glutPostRedisplay();
}

void texture_sphere_menu(int id) {
	switch (id) {
	case 1:
		textureSphereFlag = 0;
		break;
	case 2:
		textureSphereFlag = 1;
		break;
	case 3:
		textureSphereFlag = 2;
		break;
	}
	glutPostRedisplay();
}

void fireworks_menu(int id) {
	switch (id) {
	case 1:
		fireworksFlag = 0;
		break;
	case 2:
		fireworksFlag = 1;
		break;
	}
	glutPostRedisplay();
}

//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
int main(int argc, char **argv)
{ int err;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    // glutInitContextVersion(3, 2);
    // glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("Homework 3");

  /* Call glewInit() and error checking */
  err = glewInit();
  if (GLEW_OK != err)
  { printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
    exit(1);
  }
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	int shadow_sub_menu = glutCreateMenu(shadow_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int lighting_sub_menu = glutCreateMenu(lighting_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int shading_sub_menu = glutCreateMenu(shading_menu);
	glutAddMenuEntry("Flat Shading", 1);
	glutAddMenuEntry("Smooth Shading", 2);

	int light_source_sub_menu = glutCreateMenu(light_source_menu);
	glutAddMenuEntry("Spot Light", 1);
	glutAddMenuEntry("Point Source", 2);

	int fog_sub_menu = glutCreateMenu(fog_menu);
	glutAddMenuEntry("No Fog", 1);
	glutAddMenuEntry("Linear", 2);
	glutAddMenuEntry("Exponential", 3);
	glutAddMenuEntry("Exponential Square", 4);

	int shadow_blending_sub_menu = glutCreateMenu(shadow_blending_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int texture_ground_sub_menu = glutCreateMenu(texture_ground_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int texture_sphere_sub_menu = glutCreateMenu(texture_sphere_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes - Contour Lines", 2);
	glutAddMenuEntry("Yes - Checkerboard", 3);

	int fireworks_sub_menu = glutCreateMenu(fireworks_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	glutCreateMenu(menu);
	glutAddMenuEntry("Default View Point", 2);
	glutAddSubMenu("Shadow", shadow_sub_menu);
	glutAddSubMenu("Enable Lighting", lighting_sub_menu);
	glutAddMenuEntry("Wire Frame Sphere", 3);
	glutAddSubMenu("Shading", shading_sub_menu);
	glutAddSubMenu("Light Source", light_source_sub_menu);
	glutAddSubMenu("Fog Options", fog_sub_menu);
	glutAddSubMenu("Blending Shadow", shadow_blending_sub_menu);
	glutAddSubMenu("Texture Mapped Ground", texture_ground_sub_menu);
	glutAddSubMenu("Texture Mapped Sphere", texture_sphere_sub_menu);
	glutAddSubMenu("Firework", fireworks_sub_menu);
	glutAddMenuEntry("Quit", 1);
	glutAttachMenu(GLUT_LEFT_BUTTON);

    init();
    glutMainLoop();
    return 0;
}
