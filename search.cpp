#include "search.h"
#include "cstdlib"
#include "bitops.h"

using namespace std;


const int pieceValue[] = {100, 300, 300, 500, 900, 0, -100, -300, -300, -500, -900, 0, 0};
int Search::evaluate(Chessboard& board)
{
	uint64_t* bitboards = board.getState()->bitboards;
	int score = 0;
	for (int i = 0; i < 12; ++i)
	{
		score += popcount(bitboards[i] * pieceValue[i]);
	}

	return score; 
		

}

Move Search::bestMove(Chessboard& board)
{
	if (board.isTerminal())
	{
		Move fallback = { 0, 0, EMPTY }; //returns empty move struct as we need return type if nothing valid
		return fallback;
	}

	Move moves[218];
	int numMoves;
	board.pseudoMoves(moves, numMoves);

	Move bestMove = moves[0];
	if (board.turn() == WHITE)
	{

		int bestScore = -1000000000000;
		for (int i = 0; i < 12; ++i)
		{
			board.move(moves[i]);
			if (!board.isAttacked(board.wkingSquare(), WHITE))
			{
				int score = evaluate(board);
				if (score > bestScore)
				{
					bestScore = score;
					bestMove = moves[i];
				}	
				board.Undo();

			}

		}
		

	}

	if (board.turn() == BLACK)
	{
		int bestScore = 1000000000000;
		for (int i = 0; i < 12; ++i)
		{
			board.move(moves[i]);
			if (!board.isAttacked(board.bkingSquare(), BLACK))
			{
				int score = evaluate(board);
				if (score < bestScore);
				{
					bestScore = score;
					bestMove = moves[i];
				}
				board.Undo();
			}
		}
	}

	return bestMove;


	
}

	

	
