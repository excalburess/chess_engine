
#include "chessboard.h"
#include "bitops.h"
#include <math.h>

using namespace std;



uint8_t Chessboard::getPiece(uint8_t square)
{
	uint64_t mask = uint64_t(1) << square; //represents the square we want the piece from
	for (int i = 0; i < 12; ++i)
		if (stateStack[stackIndex].bitboards[i] & mask) //bitwise and to check 0 or 1 for occupation || Go to slot stackIndex in the stack, take its bitboards array, take element i of that array, and OR mask into it
		{
			return i;
		}
	return EMPTY; //return empty otherwise

}

void Chessboard::setPiece(uint8_t piece, uint8_t square)
{
	uint64_t mask = uint64_t(1) << square;
	for (int i = 0; i < 12; ++i)
		if (i == piece)  //or-ing it will put a 1 in square where piece is
		{
			stateStack[stackIndex].bitboards[i] |= mask;
		}
		else //set to 0
		{
			stateStack[stackIndex].bitboards[i] &= ~mask; 
		}

}

uint8_t Chessboard::turn()
{
	return stateStack[stackIndex].turn;
}


void Chessboard::move(const Move& move)
{
	//take in the coordinates: 
	stateStack[stackIndex + 1] = stateStack[stackIndex];
	stackIndex++;
	//update top of stack with the current state

	uint64_t fromBoard = uint64_t(1) << move.from; //1 in from 
	uint64_t toBoard = uint64_t(1) << move.to; // 1 on board to move to
	uint64_t moveBoard = fromBoard | toBoard; //1 in both to and from squares

	uint8_t movedPiece = EMPTY;

	//moving a piece
	for (int i = stateStack[stackIndex].turn == WHITE ? 0 : 6; i < (stateStack[stackIndex].turn == WHITE ? 6 : 12); ++i)
	{
		if (stateStack[stackIndex].bitboards[i] & fromBoard) 
		{
			stateStack[stackIndex].bitboards[i] ^= moveBoard; 
			movedPiece = i;
			break;
		}

	}

	//piece capture
	for (int i = stateStack[stackIndex].turn == WHITE ? 6 : 0; i < (stateStack[stackIndex].turn == WHITE ? 11 : 5); ++i)
	{
		if (stateStack[stackIndex].bitboards[i] & toBoard)
		{
			stateStack[stackIndex].bitboards[i] ^= toBoard;
			break;
		}
		
	}

	//enpassant capture
	if (toBoard == stateStack[stackIndex].enpassantTarget)
	{
		if (movedPiece == WHITE_PAWN)
		{
			stateStack[stackIndex].bitboards[BLACK_PAWN] ^= stateStack[stackIndex].enpassantTarget << 8;
			
		}
		else if (movedPiece == BLACK_PAWN)
		{
			stateStack[stackIndex].bitboards[WHITE_PAWN] ^= stateStack[stackIndex].enpassantTarget >> 8;
		}
	}

	//castling
	if (movedPiece == WHITE_KING)
	{
		if (moveBoard == 0x5000000000000000)
		{
			stateStack[stackIndex].bitboards[WHITE_ROOK] ^= 0xa000000000000000;
		}
		else if (moveBoard == 0x1400000000000000)
		{
			stateStack[stackIndex].bitboards[WHITE_ROOK] ^= 0x0900000000000000;
		}
	}
	else if (movedPiece == BLACK_KING)
	{
		if (moveBoard == 0x000000000000050) {
			stateStack[stackIndex].bitboards[BLACK_ROOK] ^= 0x00000000000000a0;
		}
		else if (moveBoard == 0x000000000000014)
		{
			stateStack[stackIndex].bitboards[BLACK_ROOK] ^= 0x0000000000000009;
		}
	}

	//promotion
	if (move.promotion != EMPTY)
	{
		stateStack[stackIndex].bitboards[stateStack[stackIndex].turn == WHITE ? WHITE_PAWN : BLACK_PAWN] ^= toBoard; //clears pawn bit at destination square
		stateStack[stackIndex].bitboards[move.promotion] ^= toBoard;	//sets the bit to promoted piece @ the same square.
	}

	//resets castling rights
	if (moveBoard & 0x9000000000000000) stateStack[stackIndex].WKC = false;
	if (moveBoard & 0x1100000000000000) stateStack[stackIndex].WQC = false;
	if (moveBoard & 0x0000000000000090) stateStack[stackIndex].BKC = false;
	if (moveBoard & 0x0000000000000011) stateStack[stackIndex].BQC = false;
		
	//enpassant target
	if ((movedPiece == WHITE_PAWN || movedPiece == BLACK_PAWN) && abs(move.from - move.to) == 16)
	{
		stateStack[stackIndex].enpassantTarget = uint64_t(1) << int((move.from + move.to) * 0.5);  //uint64_t(1) sets 1 halfway square between move from and to on the "enpassant target bitboard"
	}
	else
	{
		stateStack[stackIndex].enpassantTarget = 0;
	}

	//turn management
	if (stateStack[stackIndex].turn == WHITE)
	{
		stateStack[stackIndex].turn = BLACK;
	}
	else
	{
		stateStack[stackIndex].turn = WHITE;
	}
}

void Chessboard::Undo()
{
	if (stackIndex > 0)
	{
		--stackIndex;
	}
}

void Chessboard::pseudoMoves(Move* moves, int& numMoves)
{
	numMoves = 0;

	//bitboards to track move possibilities
	uint64_t whiteboard = 0;
	uint64_t blackboard = 0;

	for (int i = 0; i < 6; ++i)
	{
		whiteboard |= stateStack[stackIndex].bitboards[i];
	}
	for (int i = 6; i < 12; ++i)
	{
		blackboard |= stateStack[stackIndex].bitboards[i];
	}

	uint64_t board = whiteboard | blackboard;

	//white move generation
	if (stateStack[stackIndex].turn == WHITE)
	{
		//pawn move generation
		uint64_t wpb = (stateStack[stackIndex].bitboards[WHITE_PAWN] >> 8) & ~board; //1 on bitboard where can move and 0 where occupied (in front of pawn move 1 check)
		uint8_t square;
		while (wpb)
		{
			square = lsbIndex(wpb); //finds a set bit at the lowest location (doesnt matter on position)
			wpb &= wpb - 1; // clear lowest set bit leave everything else to move to next pawn
			uint8_t origin = square + 8; //keep track or origin
			moves[numMoves++] = { origin, square, EMPTY }; //(maps to move struct with from to and promotion) (creates an new entry on stack)
		}
	}

	//black move generation
	else
	{
		uint64_t bpb = (stateStack[stackIndex].bitboards[BLACK_PAWN] << 8) & ~board;
		uint8_t square;
		while (bpb)
		{
			square = lsbIndex(bpb);
			bpb &= bpb - 1;
			uint8_t origin = square - 8;
			moves[numMoves++] = { origin, square, EMPTY };
		}
	}


}

bool Chessboard::isLegal(const Move& move)
{
	Move moves[218];  //generate a moves buffer a bit like the stack index
	int NumMoves; //keeps track of which moves 
	pseudoMoves(moves, NumMoves);

	for (int i = 0; i < NumMoves; ++i)
	{
		if (move == moves[i])  //checks if move we are checking matches any of the pseudoMoves (compiler cant check if two instances of moves are different) -> need overloading 
			{
			return i;
			} 
	}

	return false;
}


Chessboard::Chessboard()
{

	stateStack = new BoardState[1000];
	stackIndex = 0;


	//initialises top of stack for bitboards
	stateStack[0].bitboards[WHITE_PAWN]   =	0x00ff000000000000; //hex representation (second row filled at each position)
	stateStack[0].bitboards[WHITE_KNIGHT] = 0x4200000000000000;
	stateStack[0].bitboards[WHITE_BISHOP] = 0x2400000000000000; //goes as such : imagine this is a row:
	stateStack[0].bitboards[WHITE_ROOK]   =	0x8100000000000000; //	   84218421
	stateStack[0].bitboards[WHITE_QUEEN]  = 0x0800000000000000; //     00000000
	stateStack[0].bitboards[WHITE_KING]	=   0x1000000000000000;
	stateStack[0].bitboards[BLACK_PAWN]	=	0x000000000000ff00;
	stateStack[0].bitboards[BLACK_KNIGHT] =	0x0000000000000042;
	stateStack[0].bitboards[BLACK_BISHOP] =	0x0000000000000024;
	stateStack[0].bitboards[BLACK_ROOK]	=	0x0000000000000081;
	stateStack[0].bitboards[BLACK_QUEEN]=	0x0000000000000008;
	stateStack[0].bitboards[BLACK_KING]	=	0x0000000000000010;

	stateStack[0].enpassantTarget = 0;

	stateStack[0].BKC = false;
	stateStack[0].WQC = false;
	stateStack[0].WKC = false;
	stateStack[0].BQC = false;

	stateStack[0].turn = WHITE;

}

Chessboard::~Chessboard()
{
	delete[] stateStack;
}

