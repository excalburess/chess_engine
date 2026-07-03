
#include <cstdint>

//object macros for easier notation in code

#define WHITE_PAWN 0 //object like macros need to be upper case and map input to an output in pre-process stage
#define WHITE_KNIGHT 1
#define WHITE_BISHOP 2
#define WHITE_ROOK 3
#define WHITE_QUEEN 4
#define WHITE_KING 5
#define BLACK_PAWN 6
#define BLACK_KNIGHT 7
#define BLACK_BISHOP 8
#define BLACK_ROOK 9
#define BLACK_QUEEN 10
#define BLACK_KING 11
#define EMPTY 12


class Chessboard
{
private:
	std::uint64_t bitboards[12]; //most efficient storage for chess board (over [][]). 1 and certain bit = occupation of specific piece, generic long int doesnt guarantee 64 bit
public: 
	//set and get piece
	std::uint8_t getPiece(std::uint8_t square);
	void setPiece(std::uint8_t piece, std::uint8_t square);

	//default constructor (defines chessboard starting state) no return type
	Chessboard();
	
};