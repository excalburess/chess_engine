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

int Search::mini(Chessboard& board, int depth)
{
	int terminal = board.isTerminal();
	if (terminal)
	{
		if (terminal == WHITE) return -1000000;
		if (terminal == BLACK) return 1000000;
		return 0;
	}
	
	if (depth <= 0)
	{
		return evaluate(board);
	}

	Move moves[218];
	int numMoves;
	board.pseudoMoves(moves, numMoves);

	//search all moves
	if (board.turn() == WHITE)
	{
		int bestScore = -10000000;
		for (int i = 0; i < numMoves; ++i)
		{
			board.move(moves[i]);
			if (!board.isAttacked(board.wkingSquare(), WHITE))
			{
				int score = mini(board, depth - 1);
				if (score > bestScore)
				{
					bestScore = score;
				}
				

			}
			board.Undo();
			
		}
		return bestScore;
	}
	else
	{
		int bestScore = 10000000;
		for (int i = 0; i < numMoves; ++i)
		{
			board.move(moves[i]);
			if (!board.isAttacked(board.bkingSquare(), BLACK))
			{
				int score = mini(board, depth - 1);
				if (score < bestScore)
				{
					bestScore = score;
				}
				

			}
			board.Undo();

			
		}
		return bestScore;
	}	 
}


Move Search::bestMove(Chessboard& board, int depth)
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
				int score = mini(board, depth - 1);
				if (score > bestScore)
				{
					bestScore = score;
					bestMove = moves[i];
				}	
				

			}
			board.Undo();

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
				int score = mini(board, depth - 1);
				if (score < bestScore);
				{
					bestScore = score;
					bestMove = moves[i];
				}
				
			}
			board.Undo();
		}
	}

	return bestMove;


	
}

	

	
