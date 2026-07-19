
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

//interfaces w turns easier

#define WHITE 13
#define BLACK 14

struct Move
{
	std::uint8_t from;
	std::uint8_t to;
	std::uint8_t promotion;

	//operator overloading
	bool operator== (const Move& rhs) const
	{
		return from == rhs.from && to == rhs.to && promotion == rhs.promotion;
	}
};



struct BoardState //access specifier = public, any member of struct accessible outside of struct unless we make members private
{
	//bitboard for every piece
	std::uint64_t bitboards[12];

	//management of the castle process
	bool WKC, BKC, WQC, BQC;

	//management of enpassant targets
	std::uint64_t enpassantTarget;
		
	//turn choosing
	std::uint8_t turn;

};


class Chessboard
{
private:
	
	BoardState* stateStack;
	int stackIndex;

public: 
	//set and get piece
	std::uint8_t getPiece(std::uint8_t square);
	void setPiece(std::uint8_t piece, std::uint8_t square);	

	std::uint8_t turn();

	//move and undo
	void move(const Move& move); //pass by const reference (address of move) 
	void Undo();

	//move generation methods
	void pseudoMoves(Move* moves, int& numMoves); //quicker to generate pseudo legal moves then filter out.
	bool isLegal(const Move& move);


	//default constructor (defines chessboard starting state) no return type
	Chessboard();

	//destructor
	~Chessboard();
	
};