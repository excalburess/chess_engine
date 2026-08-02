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

	//if move is legal make it
	while (numMoves)
	{
		int r = rand() % numMoves;
		board.move(moves[r]);
		bool illegal = board.turn() == WHITE ? board.isAttacked(board.bkingSquare(), BLACK) : board.isAttacked(board.wkingSquare(), WHITE);
		board.Undo();
		if (!illegal) return moves[r];
		--numMoves;
		moves[r] = moves[numMoves];
	}
}

	

	
