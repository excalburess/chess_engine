	//good practice to put function code for class methods in seperate c++ file
	//
	// explanation of bitboard
	//
	// 0 0 0 0 0 0 0 0										 0  1  2  3  4  5  6  7
	// 0 0 0 0 0 0 0 0										 8  9 10 11 12 13 14 15
	// 0 0 0 0 0 0 0 0									    16 17 18 19 20 21 22 23
	// 0 0 0 0 1 0 0 0 -> 1 indicates chess piece			24 25 26 27 28 29 30 31
	// 0 0 0 0 0 0 0 0										32 33 34 35 36 37 38 39
	// 0 0 0 0 0 0 0 0										40 41 42 43 44 45 46 47
	// 0 0 0 0 0 0 0 0										48 49 50 51 52 53 54 55
	// 0 0 0 0 0 0 0 0										56 57 58 59 60 61 62 63
	//
	//above is stored in a uint64_t int 
	//to get a piece, mask each offset by the square i.e. 1,2,3,4....
	//and left shift to


	#include "chessboard.h"

	using namespace std;

	uint8_t Chessboard::getPiece(uint8_t square)
	{
		uint64_t mask = uint64_t(1) << square; //represents the square we want the piece from
		for (int i=0; i < 12; ++i)
			if (bitboards[i] & mask) //bitwise and to check 0 or 1 for occupation
			{
				return i;
			}
		return EMPTY; //return empty otherwise
		
	}
	//:: specifies the class to which the member being defined belongs, granting exactly the same scope properties as if this function definition was directly included within the class definition.
	void Chessboard::setPiece(uint8_t square, uint8_t piece)
	{
		uint64_t mask = uint64_t(1) << square;
		for (int i = 0; i < 12; ++i)
			if (i == piece)  //or-ing it will put a 1 in square where piece is
			{
				bitboards[i] |= mask; 
			}
			else //set to 0
			{
				bitboards[i] &= ~mask; // ~ = not and &= shortform for anding so a = a|b is equal to a |= b
			} 

	}

	Chessboard::Chessboard()
	{
		bitboards[WHITE_PAWN]   =	0x00ff000000000000; //hex representation (second row filled at each position)
		bitboards[WHITE_KNIGHT] =   0x2400000000000000;
		bitboards[WHITE_BISHOP] =   0x4200000000000000; //goes as such : imagine this is a row:
		bitboards[WHITE_ROOK]   =	0x8100000000000000; //	   84218421
		bitboards[WHITE_QUEEN]  =   0x0800000000000000; //     00000000
		bitboards[WHITE_KING]	=   0x1000000000000000;
		bitboards[BLACK_PAWN]	=	0x000000000000ff00;
		bitboards[BLACK_KNIGHT] =	0x0000000000000024;
		bitboards[BLACK_BISHOP] =	0x0000000000000042;
		bitboards[BLACK_ROOK]	=	0x0000000000000081;
		bitboards[BLACK_QUEEN]	=	0x0000000000000008;
		bitboards[BLACK_KING]	=	0x0000000000000010;

	}