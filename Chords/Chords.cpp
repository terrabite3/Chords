// Chords.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifndef WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>
#endif

#include <math.h>
const double PI = 3.14159265358979323846;

#include <GL/glew.h>
#include <gl/glut.h>
#include <GL/freeglut.h>

#include <vector>
#include <iostream>
#include <fstream>

const char* VERTEX_SHADER = R"(#version 330 core
// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
// Output data ; will be interpolated for each fragment.
out vec2 UV;
void main() {
	gl_Position = vec4(vertexPosition_modelspace, 1);
	UV = (vertexPosition_modelspace.xy + vec2(1, 1)) / 2.0;
})";

const char* FRAGMENT_SHADER = R"(#version 330 core
// Ouput data
layout(location = 0) out vec4 color;
uniform sampler2D mytexture;
in vec2 UV;
void main(){
	color = texture(mytexture, UV);
})";

GLuint LoadShaders(/*const char * vertex_file_path, const char * fragment_file_path*/) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	glShaderSource(VertexShaderID, 1, &VERTEX_SHADER, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	glShaderSource(FragmentShaderID, 1, &FRAGMENT_SHADER, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}



// Parameters
int FRAMERATE, WIDTH, HEIGHT, FB_WIDTH, FB_HEIGHT, CIRCLE_SEGMENTS, NUM_LINES;
double PRODUCT, PRODUCT_DELTA, MARGIN_FACTOR;


void setColorHsv(double h, double s, double v)
{
	double      hh, p, q, t, ff;
	double r, g, b;
	long        i;

	if (s <= 0.0) {       // < is bogus, just shuts up warnings
		glColor3f(v, v, v);
		return;
	}
	hh = h * 360;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = v * (1.0 - s);
	q = v * (1.0 - (s * ff));
	t = v * (1.0 - (s * (1.0 - ff)));

	switch (i) {
	case 0:
		glColor3f(v, t, p);
		return;
	case 1:
		glColor3f(q, v, p);
		return;
	case 2:
		glColor3f(p, v, t);
		return;

	case 3:
		glColor3f(p, q, v);
		return;
	case 4:
		glColor3f(t, p, v);
		return;
	case 5:
	default:
		glColor3f(v, p, q);
		return;
	}
}


double mod(double number, double modulo) {

	number -= floor(number / modulo) * modulo;

	if (number < 0 || number >= modulo)
		printf("Error\n");
	return number;
}

void doInterestingStuff() {
	PRODUCT += PRODUCT_DELTA;

	// Draw the lines
	glLineWidth(0.01);
	glBegin(GL_LINES);

	for (int i = 0; i < NUM_LINES; ++i) {
		double theta0 = 1.0 * i / NUM_LINES;
		double x0 = sin(2 * PI * theta0) * MARGIN_FACTOR;
		double y0 = cos(2 * PI * theta0) * MARGIN_FACTOR;

		double theta1 = i * PRODUCT / NUM_LINES;
		double x1 = sin(2 * PI * theta1) * MARGIN_FACTOR;
		double y1 = cos(2 * PI * theta1) * MARGIN_FACTOR;

		setColorHsv(mod(theta1, 1.0), 1, 1);

		glVertex2f(x0, y0);
		glVertex2f(x1, y1);
	}

	glEnd();


	// Draw the circle
	glLineWidth(2);
	glBegin(GL_LINE_LOOP);
	glColor3f(1, 1, 1);
	for (int i = 0; i < CIRCLE_SEGMENTS; ++i) {
		double theta = i * 2 * PI / CIRCLE_SEGMENTS;
		double x = cos(theta) * MARGIN_FACTOR;
		double y = sin(theta) * MARGIN_FACTOR;
		glVertex2f(x, y);
	}
	glEnd();
}

GLuint FramebufferName, renderedTexture, quad_programID, texID, timeID, quad_vertexbuffer;

void setup() {

	glClearColor(0, 0, 0, 1);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// Create a framebuffer object to hold the texture
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// Create a texture to render to
	glGenTextures(1, &renderedTexture);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	// Generate an empty image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FB_WIDTH, FB_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Set the correct filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Set "renderedTexture" as our color attachment #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

	// Set the list of draw buffers
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	// Check that the framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		exit(1);


	GLuint quad_VertexArrayID;
	glGenVertexArrays(1, &quad_VertexArrayID);
	glBindVertexArray(quad_VertexArrayID);

	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
	};

	quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	quad_programID = LoadShaders(/*"Passthrough.vertexshader", "SimpleTexture.fragmentshader"*/);
	texID = glGetUniformLocation(quad_programID, "mytexture");

}
void display() {

	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glViewport(0, 0, FB_WIDTH, FB_HEIGHT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(0);

	doInterestingStuff();


	// Render the framebuffer to the screen

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport((WIDTH - HEIGHT) / 2, 0, HEIGHT, HEIGHT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the shader for rendering the framebuffer
	glUseProgram(quad_programID);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	glUniform1i(texID, 0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);

	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

	glDisableVertexAttribArray(0);

	glutSwapBuffers();
}

void timer(int value) {
	glutPostRedisplay();
	glutTimerFunc(1000 / FRAMERATE, timer, 0);
}

double savedDelta;
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case ' ':
		if (PRODUCT_DELTA != 0) {
			savedDelta = PRODUCT_DELTA;
			PRODUCT_DELTA = 0;
		}
		else {
			PRODUCT_DELTA = savedDelta;
		}
		break;

	case 'r':
		PRODUCT_DELTA = -PRODUCT_DELTA;
		savedDelta = -savedDelta;
		break;

	case 'b':
		printf("Breaking\n");
		break;

	case 'i':
		printf("Product:\t%f\tDelta:\t%f\n", PRODUCT, PRODUCT_DELTA);
		break;

	case 27:	// Escape
		exit(0);

		break;

	case GLUT_KEY_RIGHT:
		if (PRODUCT_DELTA == 0)
			PRODUCT += abs(savedDelta);
		else if (PRODUCT_DELTA > 0)
			PRODUCT = floor(PRODUCT) + 1;
		else
			PRODUCT = ceil(PRODUCT) + 1;
		break;

	case GLUT_KEY_LEFT:
		if (PRODUCT_DELTA == 0)
			PRODUCT -= abs(savedDelta);
		else if (PRODUCT_DELTA > 0)
			PRODUCT = floor(PRODUCT) - 1;
		else
			PRODUCT = ceil(PRODUCT) - 1;
		break;

	case GLUT_KEY_UP:
		PRODUCT_DELTA *= 1.1;
		break;

	case GLUT_KEY_DOWN:
		PRODUCT_DELTA /= 1.1;
		break;

	default:
		break;
	}
}

void special(int key, int x, int y) {
	keyboard((unsigned char)key, x, y);
}

int main(int argc, char *argv[])
{
	// Setup parameters
	FRAMERATE = 60;
	WIDTH = 1080;
	HEIGHT = 1080;


	//NUM_LINES = (int)(200 * PI);
	NUM_LINES = 1000;
	PRODUCT = 0;
	PRODUCT_DELTA = 0.001;

	CIRCLE_SEGMENTS = 200;

	MARGIN_FACTOR = 0.95;


	glutInit(&argc, argv);

	WIDTH = glutGet(GLUT_SCREEN_WIDTH);
	HEIGHT = glutGet(GLUT_SCREEN_HEIGHT);

	double framebuffer_scale = 2;
	FB_WIDTH = (int)(HEIGHT * framebuffer_scale);
	FB_HEIGHT = (int)(HEIGHT * framebuffer_scale);


	glutSetOption(GLUT_MULTISAMPLE, 2);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Chords");

	glutFullScreen();

	glewInit();

	setup();
	glutDisplayFunc(display);
	glutTimerFunc(1000 / FRAMERATE, timer, 0);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);

	glutMainLoop();

    return 0;
}

