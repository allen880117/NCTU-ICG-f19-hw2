#include "FreeImage.h"
#include "glew.h"
#include "glut.h"
#include "shader.h"
#include <iostream>
#include <string>
#include <cmath>
#define PI 3.14159265358
using namespace std;

int windowSize[2] = { 600, 600 };
void display();
void idle();
void reshape(GLsizei w, GLsizei h);
void keyboard(unsigned char key, int x, int y);

GLuint program;
GLuint vboName;

//Initialize the shaders 
void shaderInit();
void textureInit();

// Texture
GLuint texture;

//Number of vertices that will be sent to shader
int verticeNumber = 3;

//Earth Variables
int earthRadius = 1;
int earthSlice = 360;
int earthStack = 180;
float earthTime = 0.0f;
float rotateAngle = 1.0f;
float earthTimeStep = 0.025f;

//Storing vertex datas that will be sent to shader
class VertexAttribute {
public:
	GLfloat position[3];
	GLfloat texcoord[2];
	void setPosition(float x, float y, float z) {
		position[0] = x;
		position[1] = y;
		position[2] = z;
	};
	void setCoord(float x, float y) {
		texcoord[0] = x;
		texcoord[1] = y;
	}
};

//Collecting vertex datas
VertexAttribute *drawTriangle();
VertexAttribute *shaderSphere(int slice, int stack);
void drawShaderSphere(int slice, int stack);

int main(int argc, char** argv) {	
	glutInit(&argc, argv);
	glutInitWindowSize(windowSize[0], windowSize[1]);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("ComputerGraphicsDemo");

	glewInit();
	shaderInit();
	// === WA === Texture Initialize
	textureInit();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutMainLoop();
	return 0;
}

# if 0
void shaderInit() {
	GLuint vert = createShader("Shaders/example.vert", "vertex");
	GLuint frag = createShader("Shaders/example.frag", "fragment");
	program = createProgram(vert, frag);

	//Generate a new buffer object
	glGenBuffers(1, &vboName);
	glBindBuffer(GL_ARRAY_BUFFER, vboName);

	//Copy vertex data to the buffer object
	VertexAttribute *vertices;
	vertices = drawTriangle();
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexAttribute) * verticeNumber, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, position)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
#endif
void shaderInit() {
	GLuint vert = createShader("Shaders/example.vert", "vertex");
	GLuint frag = createShader("Shaders/example.frag", "fragment");
	program = createProgram(vert, frag);

	//Generate a new buffer object
	glGenBuffers(1, &vboName);
	glBindBuffer(GL_ARRAY_BUFFER, vboName);

	//Copy vertex data to the buffer object
	VertexAttribute *vertices;
	vertices = shaderSphere(earthSlice, earthStack);
	verticeNumber = earthSlice * (earthStack + 1) * 2;
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexAttribute) * verticeNumber, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, position)));
	
	// =TEXTURE = Add another VertexAttribPointer to point to the vertex coordinate
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, texcoord)));
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void textureInit() { // =TEXTURE= //
	// Enable Texture
	glEnable(GL_TEXTURE_2D);
	
	// Load the Texture
	FIBITMAP* pImage = FreeImage_Load(FreeImage_GetFileType("earth_texture_map.jpg", 0), "earth_texture_map.jpg");
	FIBITMAP* p32BitsImage = FreeImage_ConvertTo32Bits(pImage);
	int iWidth = FreeImage_GetWidth(p32BitsImage);
	int iHeight = FreeImage_GetHeight(p32BitsImage);

	// Generate Texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// Set the Texture warping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set the texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Specifies a texture environmentz
	glTexEnvf(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Generate a two-dimensional texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, iWidth, iHeight, 0, 
		GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage));

	glGenerateMipmap(GL_TEXTURE_2D);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	FreeImage_Unload(p32BitsImage);
	FreeImage_Unload(pImage);
}

void display() {   
	// viewing and modeling transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 3.0f,// eye
		0.0f, 0.0f, 0.0f,// center
		0.0f, 1.0f, 0.0f);// up

	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (GLfloat)512 / (GLfloat)512, 1, 500);

	// viewport transformation
	glViewport(0, 0, windowSize[0], windowSize[1]);

	//Clear the buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// === WA === Rotate Sphere
	glMatrixMode(GL_MODELVIEW);
	glRotatef(rotateAngle * earthTime, 0.0f, 1.0f, 0.0f); // Along Y-axis

	GLfloat pmtx[16];
	GLfloat mmtx[16];
	glGetFloatv(GL_PROJECTION_MATRIX, pmtx); // Get Projection-View Matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, mmtx); // Get Model-View Matrix
 	GLint pmatLoc = glGetUniformLocation(program, "Projection");
	GLint mmatLoc = glGetUniformLocation(program, "ModelView");

	// =TEXTURE= Get the location of uniform variable in fragment shader
	GLint texLoc = glGetUniformLocation(program, "earthTexture");

	glUseProgram(program);

	//input the modelview matrix into vertex shader
	glUniformMatrix4fv(pmatLoc, 1, GL_FALSE, pmtx);
	//input the rotation matrix into vertex shader
	glUniformMatrix4fv(mmatLoc, 1, GL_FALSE, mmtx);

	// =TEXTURE= Active, Bind, and Set up the value of uniform variable in fragment shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(texLoc, 0);

	// === WA === glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
	drawShaderSphere(earthSlice, earthStack); // WA_MODE
	
	glUseProgram(0);
	glPopMatrix();
	glutSwapBuffers();
}

VertexAttribute *drawTriangle() {
	VertexAttribute *vertices;
	vertices = new VertexAttribute[verticeNumber];
	vertices[0].setPosition(0,0,0);
	vertices[1].setPosition(1,0,0);
	vertices[2].setPosition(0,1,0);
	return vertices;
}

void reshape(GLsizei w, GLsizei h) {
	windowSize[0] = w;
	windowSize[1] = h;
}

void keyboard(unsigned char key, int x, int y) {

}

void idle() {
	glutPostRedisplay();
	earthTime = (earthTime + earthTimeStep > 360.0f)? 0.0f : earthTime + earthTimeStep;
}

void drawShaderSphere(int slice, int stack) {
	int step = (stack + 1) * 2;
	int counter = 0;
	for (int i = 0; i < slice; i++) {
		glDrawArrays(GL_TRIANGLE_STRIP, counter, step);
		counter += step;
	}
}

VertexAttribute *shaderSphere(int slice, int stack) {
	VertexAttribute *vertices;
	vertices = new VertexAttribute[slice*(stack+1)*2];
	int counter = 0;
	float coordXCounter = 1.0f;
	float coordYCounter = 1.0f;
	float stepX = (float)1.0f/slice;
	float stepY = (float)1.0f/(stack+1);
	double x, y, z;
	double slice_step = 2 * PI / slice, stack_step = PI / stack;
	for (int i = 0; i < slice; i++) {

		coordXCounter = 1.0f - i*stepX;
		coordYCounter = 1.0f;
		
		for (int j = 0; j < stack + 1; j++) {
			x = sin(j * stack_step) * cos(i*slice_step);
			y = cos(j * stack_step);
			z = sin(j * stack_step) * sin(i*slice_step);
			vertices[counter].setPosition(x, y, z);
			vertices[counter].setCoord(coordXCounter, coordYCounter);
			counter++;

			x = sin(j * stack_step) * cos((i + 1)*slice_step);
			y = cos(j * stack_step);
			z = sin(j * stack_step) * sin((i + 1)*slice_step);
			vertices[counter].setPosition(x, y, z);
			vertices[counter].setCoord(coordXCounter - stepX, coordYCounter);
			counter++;
			
			coordYCounter -= stepY;
			if (coordYCounter <= 0.0f) stepY = -stepY;
		}

		stepY = -stepY;
	}
	return vertices;
}
