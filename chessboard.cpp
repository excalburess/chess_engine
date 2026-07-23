
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
		uint64_t whitePawnBoard = (stateStack[stackIndex].bitboards[WHITE_PAWN] >> 8) & ~board; //1 on bitboard where can move and 0 where occupied (in front of pawn move 1 check)
		uint64_t doublePawnMoveBoard = ((whitePawnBoard & 0x0000ff0000000000) >> 8) & ~board;
		uint8_t square;
		while (whitePawnBoard)
		{
			square = lsbIndex(whitePawnBoard); //finds a set bit at the lowest location (doesnt matter on position)
			whitePawnBoard &= whitePawnBoard - 1; // clear lowest set bit leave everything else to move to next pawn
			uint8_t origin = square + 8;  //keep track or origin
			if (square < 8)
			{
				moves[numMoves++] = { origin, square, WHITE_QUEEN };
				moves[numMoves++] = { origin, square, WHITE_BISHOP };
				moves[numMoves++] = { origin, square, WHITE_KNIGHT };
				moves[numMoves++] = { origin, square, WHITE_ROOK };
			}
			else
			{
				moves[numMoves++] = { origin, square, EMPTY }; //(maps to move struct with from to and promotion) (creates an new entry on stack)
			}
		}	

		while (doublePawnMoveBoard)
		{
			square = lsbIndex(doublePawnMoveBoard);
			doublePawnMoveBoard &= doublePawnMoveBoard - 1;
			uint8_t origin = square + 16;
			moves[numMoves++] = { origin, square, EMPTY };
		}


		//pawn attacks 
		whitePawnBoard = ((stateStack[stackIndex].bitboards[WHITE_PAWN] & 0xfefefefefefefefe) >> 9) & (blackboard | stateStack[stackIndex].enpassantTarget);
		while (whitePawnBoard)
		{
			square = lsbIndex(whitePawnBoard);
			whitePawnBoard &= whitePawnBoard - 1;
			uint8_t origin = square + 9;
			if (square < 8)
			{
				moves[numMoves++] = { origin, square, WHITE_QUEEN };
				moves[numMoves++] = { origin, square, WHITE_BISHOP };
				moves[numMoves++] = { origin, square, WHITE_KNIGHT };
				moves[numMoves++] = { origin, square, WHITE_ROOK };
			}
			else
			{
				moves[numMoves++] = { origin, square, EMPTY }; //(maps to move struct with from to and promotion) (creates an new entry on stack)
			}
			
		}

		//pawn attacks 
		whitePawnBoard = ((stateStack[stackIndex].bitboards[WHITE_PAWN] & 0x7f7f7f7f7f7f7f7f) >> 7) & (blackboard | stateStack[stackIndex].enpassantTarget);
		while (whitePawnBoard)
		{
			square = lsbIndex(whitePawnBoard);
			whitePawnBoard &= whitePawnBoard - 1;
			uint8_t origin = square + 7;
			if (square < 8)
			{
				moves[numMoves++] = { origin, square, WHITE_QUEEN };
				moves[numMoves++] = { origin, square, WHITE_BISHOP };
				moves[numMoves++] = { origin, square, WHITE_KNIGHT };
				moves[numMoves++] = { origin, square, WHITE_ROOK };
			}
			else
			{
				moves[numMoves++] = { origin, square, EMPTY }; //(maps to move struct with from to and promotion) (creates an new entry on stack)
			}
		}


		//white knight
		uint64_t whiteKnightBitboard = stateStack[stackIndex].bitboards[WHITE_KNIGHT];
		while (whiteKnightBitboard)
		{
			square = lsbIndex(whiteKnightBitboard);
			whiteKnightBitboard &= whiteKnightBitboard - 1;

			uint64_t knightMoves = knightAttack[square] & ~whiteboard; //1 on every square where no white piece
			uint8_t target;
			while (knightMoves)
			{
				target = lsbIndex(knightMoves);
				knightMoves &= knightMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}


		}

		//diagonal moves

		uint64_t dbb = stateStack[stackIndex].bitboards[WHITE_BISHOP] | stateStack[stackIndex].bitboards[WHITE_QUEEN];
		while (dbb)
		{
			square = lsbIndex(dbb);
			dbb &= dbb - 1;

			uint64_t diagonalMoves = 0;
			for (int i = 0; i < 4; ++i)
			{
				diagonalMoves |= diagonalAttacks[square][i]; //extract piece positions
				uint64_t blockedPieces = board & diagonalAttacks[square][i];
				if (blockedPieces)
				{
					uint8_t blockedIndex;
					if (i == 0 || i == 3)
					{
						blockedIndex = lsbIndex(blockedPieces);
					}
					else
					{
						blockedIndex = msbIndex(blockedPieces);
					}
					diagonalMoves &= ~diagonalAttacks[blockedIndex][i];
				}
			}
			diagonalMoves &= ~whiteboard;


			uint8_t target;
			while (diagonalMoves)
			{
				target = lsbIndex(diagonalMoves);
				diagonalMoves &= diagonalMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}
		}


		//white king
		square = lsbIndex(stateStack[stackIndex].bitboards[WHITE_KING]);
		uint64_t whiteKingBoard = kingAttack[square] & ~whiteboard;
		while(whiteKingBoard)
		{
			uint8_t target = lsbIndex(whiteKingBoard);
			whiteKingBoard &= whiteKingBoard - 1;
			moves[numMoves++] = { square, target, EMPTY };
		}

	}

	//black move generation
	else
	{
		uint64_t blackPawnBoard = (stateStack[stackIndex].bitboards[BLACK_PAWN] << 8) & ~board;
		uint64_t doubleBlackPawnMoveBoard = ((blackPawnBoard & 0x0000000000ff0000) << 8) & ~board;
		uint8_t square;
		while (blackPawnBoard)
		{
			square = lsbIndex(blackPawnBoard);
			blackPawnBoard &= blackPawnBoard - 1;
			uint8_t origin = square - 8;
			if (square >= 56)
			{
				moves[numMoves++] = { origin, square, BLACK_QUEEN };
				moves[numMoves++] = { origin, square, BLACK_BISHOP };
				moves[numMoves++] = { origin, square, BLACK_KNIGHT };
				moves[numMoves++] = { origin, square, BLACK_ROOK };
			}
			else
			{
				moves[numMoves++] = { origin, square, EMPTY }; //(maps to move struct with from to and promotion) (creates an new entry on stack)
			}
		}

		while (doubleBlackPawnMoveBoard)
		{
			square = lsbIndex(doubleBlackPawnMoveBoard);
			doubleBlackPawnMoveBoard &= doubleBlackPawnMoveBoard - 1;
			uint8_t origin = square - 16;
			moves[numMoves++] = { origin, square, EMPTY };
		}

		//pawn attacks 
		blackPawnBoard = ((stateStack[stackIndex].bitboards[BLACK_PAWN] & 0xfefefefefefefefe) << 9) & (whiteboard | stateStack[stackIndex].enpassantTarget);
		while (blackPawnBoard)
		{
			square = lsbIndex(blackPawnBoard);
			blackPawnBoard &= blackPawnBoard - 1;
			uint8_t origin = square - 9;
			if (square >= 56)
			{
				moves[numMoves++] = { origin, square, BLACK_QUEEN };
				moves[numMoves++] = { origin, square, BLACK_BISHOP };
				moves[numMoves++] = { origin, square, BLACK_KNIGHT };
				moves[numMoves++] = { origin, square, BLACK_ROOK };
			}
			else
			{
				moves[numMoves++] = { origin, square, EMPTY }; //(maps to move struct with from to and promotion) (creates an new entry on stack)
			}
		}

		//pawn attacks 
		blackPawnBoard = ((stateStack[stackIndex].bitboards[BLACK_PAWN] & 0x7f7f7f7f7f7f7f7f) << 7) & (whiteboard | stateStack[stackIndex].enpassantTarget);
		while (blackPawnBoard)
		{
			square = lsbIndex(blackPawnBoard);
			blackPawnBoard &= blackPawnBoard - 1;
			uint8_t origin = square - 7;
			if (square >= 56)
			{
				moves[numMoves++] = { origin, square, BLACK_QUEEN };
				moves[numMoves++] = { origin, square, BLACK_BISHOP };
				moves[numMoves++] = { origin, square, BLACK_KNIGHT };
				moves[numMoves++] = { origin, square, BLACK_ROOK };
			}
			else
			{
				moves[numMoves++] = { origin, square, EMPTY };
			}
		}

		//black knight
		uint64_t blackKnightBitboard = stateStack[stackIndex].bitboards[BLACK_KNIGHT];
		while (blackKnightBitboard)
		{
			square = lsbIndex(blackKnightBitboard);
			blackKnightBitboard &= blackKnightBitboard - 1;

			uint64_t knightMoves = knightAttack[square] & ~blackboard; //1 on every square where no white piece
			uint8_t target;
			while (knightMoves)
			{
				target = lsbIndex(knightMoves);
				knightMoves &= knightMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}

		}

		uint64_t dbb = stateStack[stackIndex].bitboards[BLACK_BISHOP] | stateStack[stackIndex].bitboards[BLACK_QUEEN];
		while (dbb)
		{
			square = lsbIndex(dbb);
			dbb &= dbb - 1;

			uint64_t diagonalMoves = 0;
			for (int i = 0; i < 4; ++i)
			{
				diagonalMoves |= diagonalAttacks[square][i]; //extract piece positions
				uint64_t blockedPieces = board & diagonalAttacks[square][i];
				if (blockedPieces)
				{
					uint8_t blockedIndex;
					if (i == 0 || i == 3)
					{
						blockedIndex = lsbIndex(blockedPieces);
					}
					else
					{
						blockedIndex = msbIndex(blockedPieces);
					}
					diagonalMoves &= ~diagonalAttacks[blockedIndex][i];
				}
			}
			diagonalMoves &= ~blackboard;

			uint8_t target;
			while (diagonalMoves)
			{
				target = lsbIndex(diagonalMoves);
				diagonalMoves &= diagonalMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}
		}

		//black king
		square = lsbIndex(stateStack[stackIndex].bitboards[BLACK_KING]);
		uint64_t blackKingBoard = kingAttack[square] & ~blackboard;
		while (blackKingBoard)
		{
			uint8_t target = lsbIndex(blackKingBoard);
			blackKingBoard &= blackKingBoard - 1;
			moves[numMoves++] = { square, target, EMPTY };
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
			return true;
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



	//generating attack boards  
	for (int x1 = 0; x1 < 8; ++x1)
	{
		for (int y1 = 0; y1 < 8; ++y1)
		{
			uint8_t squareIndex = x1 + y1 * 8;
			
			//knightAttacks
			knightAttack[squareIndex] = 0;
			if (x1 < 7 && y1 < 6) knightAttack[squareIndex] |= uint64_t(1) << (x1 + 1 + 8 * (y1 + 2));
			if (x1 < 6 && y1 < 7) knightAttack[squareIndex] |= uint64_t(1) << (x1 + 2 + 8 * (y1 + 1));
			if (x1 > 0 && y1 < 6) knightAttack[squareIndex] |= uint64_t(1) << (x1 - 1 + 8 * (y1 + 2));
			if (x1 > 1 && y1 < 7) knightAttack[squareIndex] |= uint64_t(1) << (x1 - 2 + 8 * (y1 + 1));
			if (x1 < 6 && y1 > 0) knightAttack[squareIndex] |= uint64_t(1) << (x1 + 2 + 8 * (y1 - 1));
			if (x1 < 7 && y1 > 1) knightAttack[squareIndex] |= uint64_t(1) << (x1 + 1 + 8 * (y1 - 2));
			if (x1 > 1 && y1 > 0) knightAttack[squareIndex] |= uint64_t(1) << (x1 - 2 + 8 * (y1 - 1));
			if (x1 > 0 && y1 > 1) knightAttack[squareIndex] |= uint64_t(1) << (x1 - 1 + 8 * (y1 - 2));

			diagonalAttacks[squareIndex][0] = 0;
			for (int x2 = x1 + 1, y2 = y1 + 1; x2 < 8 && y2 < 8; ++x2, ++y2) diagonalAttacks[squareIndex][0] |= uint64_t(1) << (x2 + 8 * y2);
			diagonalAttacks[squareIndex][1] = 0;
			for (int x2 = x1 - 1, y2 = y1 - 1; x2 >= 0 && y2 >= 0; --x2, --y2) diagonalAttacks[squareIndex][1] |= uint64_t(1) << (x2 + 8 * y2);
			diagonalAttacks[squareIndex][2] = 0;
			for (int x2 = x1 + 1, y2 = y1 - 1; x2 < 8 && y2 >= 0; ++x2, --y2) diagonalAttacks[squareIndex][2] |= uint64_t(1) << (x2 + 8 * y2);
			diagonalAttacks[squareIndex][3] = 0;
			for (int x2 = x1 - 1, y2 = y1 + 1; x2 >= 0 && y2 < 8; --x2, ++y2) diagonalAttacks[squareIndex][3] |= uint64_t(1) << (x2 + 8 * y2);

			straightAttacks[squareIndex][0] = 0;
			for (int x2 = x1 + 1; x2 < 8; ++x2) straightAttacks[squareIndex][0] |= uint64_t(1) << (x2 + 8 * y1);
			straightAttacks[squareIndex][1] = 0;
			for (int x2 = x1 + 1; x2 >= 0; --x2) straightAttacks[squareIndex][1] |= uint64_t(1) << (x2 + 8 * y1);
			straightAttacks[squareIndex][2] = 0;
			for (int y2 = y1 + 1; y2 < 8; ++y2) straightAttacks[squareIndex][2] |= uint64_t(1) << (x1 + 8 * y2);
			straightAttacks[squareIndex][3] = 0;
			for (int y2 = y1 + 1; y2 >= 0; --y2) straightAttacks[squareIndex][3] |= uint64_t(1) << (x1 + 8 * y2);

			kingAttack[squareIndex] = 0;
			if (x1 < 7 && y1 < 7 ) kingAttack[squareIndex] |= uint64_t(1) << (x1 + 1 + 8 * (y1 + 1));
			if (x1 < 7) kingAttack[squareIndex] |= uint64_t(1) << (x1 + 1 + 8 * y1);
			if (x1 < 7 && y1 > 7) kingAttack[squareIndex] |= uint64_t(1) << (x1 + 1 + 8 * (y1 - 1));
			if (y1 < 7) kingAttack[squareIndex] |= uint64_t(1) << (x1 + 8 * (y1 + 1));
			if (y1 > 0) kingAttack[squareIndex] |= uint64_t(1) << (x1 + 8 * (y1 - 1));
			if (x1 > 0 && y1 < 7) kingAttack[squareIndex] |= uint64_t(1) << (x1 - 1 + 8 * (y1 + 1));
			if (x1 > 0) kingAttack[squareIndex] |= uint64_t(1) << (x1 - 1 + 8 * y1);
			if (x1 > 0 && y1 > 0) kingAttack[squareIndex] |= uint64_t(1) << (x1 - 1 + 8 * (y1 - 1));

		}
	}
	

}

Chessboard::~Chessboard()
{
	delete[] stateStack;
}

