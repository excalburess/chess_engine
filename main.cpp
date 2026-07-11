#include <iostream>
#include <GL/freeglut.h>
#include "chessboard.h"
#include <GL/GL.h>
#include <GL/GLU.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint pieceTexture[12];

using namespace std;

Chessboard game;

int width = 1920;
int height = 1080;

double board_x = 0.5 * (width - height);
double board_y = 0;
double board_size = height;

double square_size = board_size * 0.9 * 0.125; //square size (0.9 alters how "zoomed" board is) (90% of board size is used 10% = boarder)
double square_x = board_x + board_size * 0.05; //(to centre grid evenly the remaining 90% is split between x and y coordinates) (top left corner of chessboard)
double square_y = board_y + board_size * 0.05; //centres grid to top left of board coord. i.e if 90% used 10%/2 = 5% so board_y + board_size * 0.05 (board_y uses offset to push it)

//selection logic
int select_x = -1;
int select_y = -1;


Move moveStack[1000];
int moveIndex = 0;



//file loading
GLuint loadTexture(const char *filename)
{
	int width, height, channels;
	unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
	if (data == nullptr)
	{
		std::cout << "Failed to load texture" << filename << std::endl;
		return 0;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//generate texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//after data copy remove copy in openGL
	stbi_image_free(data);

	return textureID;
}


//sprite logic
void drawPiece(int x, int y, int size, int piece)
{
	switch (piece)
	{
	case BLACK_PAWN:
	case BLACK_KNIGHT:
	case BLACK_ROOK:
	case BLACK_BISHOP:
	case BLACK_KING:
	case BLACK_QUEEN:
	case WHITE_PAWN:
	case WHITE_KNIGHT:
	case WHITE_ROOK:
	case WHITE_BISHOP:
	case WHITE_KING:
	case WHITE_QUEEN:
		glColor3f(1.0f, 1.0f, 1.0f);
		if (square_size < 0) {
			glBindTexture(GL_TEXTURE_2D, pieceTexture[piece]); //loads texture for quad
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);               // top left
			glTexCoord2f(-1.0f, 0.0f); glVertex2f(x + size, y);        // top right
			glTexCoord2f(-1.0f, -1.0f); glVertex2f(x + size, y + size); // bottom right
			glTexCoord2f(0.0f, -1.0f); glVertex2f(x, y + size);        // bottom left
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, pieceTexture[piece]); //loads texture for quad
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);               // top left
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x + size, y);        // top right
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x + size, y + size); // bottom right
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + size);        // bottom left
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		break;
	case EMPTY:
	default:

		return;
	}
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(2.38, 2.38, 2.10);
	//glBegin(GL_QUADS);
	//glVertex2f(board_x, board_y); //bottom left
	//glVertex2f(board_x, board_y + board_size); //top left
	//glVertex2f(board_y + board_size, board_x + board_size); //top right 
	//glVertex2f(board_x + board_size, board_y); //bottom right
	//glEnd();

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

	//showing the previous move
	glLineWidth(square_size * 0.15);
	glColor3f(0.8, 0.639, 0.118);
	glBegin(GL_LINES);
	glVertex2f(square_x + square_size * (moveStack[moveIndex - 1].from % 8 + 0.5), square_y + square_size * (moveStack[moveIndex - 1].from / 8 + 0.5));
	glVertex2f(square_x + square_size * (moveStack[moveIndex - 1].to % 8 + 0.5), square_y + square_size * (moveStack[moveIndex - 1].to / 8+ 0.5));
	glEnd();




	for (int x = 0; x < 8; ++x)
		for (int y = 0; y < 8; ++y)
		{
			uint8_t square = x + y * 8;
			drawPiece(square_x + square_size * x, square_y + square_size * y, square_size, game.getPiece(square));
		}

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

//keydown decleration
void keydown(unsigned char key, int x, int y);

void resize(int w, int h) { //resizes the window for chess board
	width = w;
	height = h;

	//recalc board params if flipped to counter switching of sides
	bool isFlipped = square_size < 0;

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

	if (isFlipped) keydown(32, 0, 0);

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

//click controls 

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		//get coords of board click (square)
		int click_x = floor((x - square_x) / square_size); //This essentially shifts coords relative to grids top left corner instead of origin
		int click_y = floor((y - square_y) / square_size); //square_size = how many squares accross is it as a float. floor just truncates in case we are in middle of square


		//if piece is valid lets move it
		if (select_x < 8 && select_x >= 0 && select_y < 8 && select_y >= 0 && game.getPiece(select_x + 8 * select_y) != EMPTY)
		{																														
			if (click_x >= 0 && click_x < 8 && click_y >= 0 && click_y < 8)																																													  
			{																													
				uint8_t from = select_x + 8 * select_y;
				uint8_t to = click_x + 8 * click_y;

				moveStack[moveIndex].from = from;
				moveStack[moveIndex].to = to;
				moveStack[moveIndex].promotion = EMPTY;

				game.move(moveStack[moveIndex++]);
			}

			select_x = -1;
			select_y = -1;
		}

		//otherwise select the square that was clicked

		else

		{
			select_x = click_x;
			select_y = click_y;
		}
	}
		
}

void keydown(unsigned char key, int x, int y)
{
	if (key == 27) glutLeaveMainLoop();
	if (key == 32)
	{
		square_x += square_size * 8;
		square_y += square_size * 8;
		square_size *= -1; //square_size negative flips checkers


	}

	if (key == '\b')
	{
		game.Undo();
		--moveIndex;
	}

}


int main(int argc, char** argv) //draw function of board
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutCreateWindow("Chess_Board");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//enable transparency
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//piece loading
	pieceTexture[WHITE_PAWN] = loadTexture("wp.png");
	pieceTexture[WHITE_KNIGHT] = loadTexture("wn.png"); //wn = knight
	pieceTexture[WHITE_BISHOP] = loadTexture("wb.png");
	pieceTexture[WHITE_ROOK] = loadTexture("wr.png");
	pieceTexture[WHITE_QUEEN] = loadTexture("wq.png");
	pieceTexture[WHITE_KING] = loadTexture("wk.png"); //wk = king
	pieceTexture[BLACK_PAWN] = loadTexture("bp.png");
	pieceTexture[BLACK_KNIGHT] = loadTexture("bn.png");
	pieceTexture[BLACK_BISHOP] = loadTexture("bb.png");
	pieceTexture[BLACK_ROOK] = loadTexture("br.png");
	pieceTexture[BLACK_QUEEN] = loadTexture("bq.png");
	pieceTexture[BLACK_KING] = loadTexture("bk.png");

	//std::cout << "Queen: " << loadTexture("wq.png") << ", Pawn: " << loadTexture("wp.png") << std::endl;
	gluOrtho2D(0, width, height, 0);
	glutDisplayFunc(draw);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keydown);
	glutTimerFunc(16, timer, NULL);
	glutReshapeFunc(resize);

	
	
	glutMainLoop();
	return 0;
}