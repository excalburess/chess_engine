
#pragma once
#include "chessboard.h"

class Search
{
private:
	uint64_t nodesSearched;

	int evaluate(Chessboard& board);

	//minimax function
	int mini(Chessboard& board, int depth);

public:
	Move bestMove(Chessboard& board, int depth); 

	uint64_t getNodesSearched();
};


 