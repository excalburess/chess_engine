#include <iostream>
#include <GL/freeglut.h>
#include "chessboard.h"

using namespace std;

Chessboard game;

int width = 1920;
int height = 1080;

double board_x = 0.5 * (width - height);
double board_y = 0;
double board_size = height;

double square_size = board_size * 0.9 * 0.125; //square size (0.9 alters how "zoomed" board is)
double square_x = board_x + board_size * 0.05;
double square_y = board_y + board_size * 0.05;

//selection logic
int select_x = -1;
int select_y = -1;

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(2.38, 2.38, 2.10);
	glBegin(GL_QUADS);
	glVertex2f(board_x, board_y);
	glVertex2f(board_x + board_size, board_y);
	glVertex2f(board_x, board_y); //draws background board
	glVertex2f(board_x, board_y + board_size);
	glEnd();

	glBegin(GL_QUADS);
	for (int x = 0; x < 8; ++x)
		for (int y = 0; y < 8; ++y)
		{
			if ((x + y) % 2)
			{
				glColor3f(0.93333, 0.93333, 0.8235);					//drawing coloured squares on board
			}
			else
			{
				glColor3f(0.4627, 0.5882, 0.3372);
			}

			glVertex2f(square_x + square_size * x, square_y + square_size * y); //draws squares by at every y n x interval adding verticies (bottom left)
			glVertex2f(square_x + square_size * (x + 1), square_y + square_size * y); //topleft
			glVertex2f(square_x + square_size * (x + 1), square_y + square_size * (y + 1)); // top right
			glVertex2f(square_x + square_size * x, square_y + square_size * (y + 1)); //bottom right
		}

	glEnd();



	//highlighting square logic for selection
	if (select_x >= 0 && select_y >= 0 && select_x <= 8 && select_y <= 8)
	{
		glColor3f(0.7254901960784313, 0.792156862745098, 0.25882352941176473);
		glBegin(GL_QUADS);
		glVertex2f(square_x + square_size * select_x, square_y + square_size * select_y); //draws squares by at every y n x interval adding verticies (bottom left)
		glVertex2f(square_x + square_size * (select_x + 1), square_y + square_size * select_y); //topleft
		glVertex2f(square_x + square_size * (select_x + 1), square_y + square_size * (select_y + 1)); // top right
		glVertex2f(square_x + square_size * select_x, square_y + square_size * (select_y + 1)); //bottom right
		glEnd();
		
	}

	glutSwapBuffers();
}

void resize(int w, int h) { //resizes the window for chess board
	width = w;
	height = h;

	if (width > height)
	{
		board_size = height;
		board_x = 0.5 * (width - height);
		board_y = 0;

	}
	else {
		board_size = width;
		board_x = 0.5 * (width - height);
		board_y = 0;
	}

	square_size = board_size * 0.9 * 0.125;
	square_x = board_x + board_size * 0.05;
	square_y = board_y + board_size * 0.05;

	glViewport(0, 0, width, height);
	glLoadIdentity();
	gluOrtho2D(0, width, height, 0);
}
	



void timer(int value) 
{
	//updates display at intervals
	glutPostRedisplay();
	glutTimerFunc(16, timer, NULL);
}

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		select_x = floor((x - square_x) / square_size);
		select_y = floor((y - square_y) / square_size);
	}
		
}

void keydown(unsigned char key, int x, int y)
{
	if (key == 27) glutLeaveMainLoop();
}


int main(int argc, char** argv) //draw function of board
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutCreateWindow("Chess_Board");
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gluOrtho2D(0, width, height, 0);
	glutDisplayFunc(draw);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keydown);
	glutTimerFunc(16, timer, NULL);
	glutReshapeFunc(resize);
	glutMainLoop();
	return 0;
}