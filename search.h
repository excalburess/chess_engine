
#pragma once
#include "chessboard.h"

class Search
{
private:
	int evaluate(Chessboard& board);
public:
	Move bestMove(Chessboard& board); 
};


 