
#pragma once
#include "chessboard.h"

class Search
{
private:
	int evaluate(Chessboard& board);

	//minimax function
	int mini(Chessboard& board, int depth);

public:
	Move bestMove(Chessboard& board, int depth); 
};


 