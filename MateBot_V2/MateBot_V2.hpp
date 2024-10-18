//(c) MateBot_V2, Chess Engine
//(c) Mate Csapo; August 2023
//Header File

//***PREAMBLE***
//------------------------------------------------------------------------------------------------
using namespace std;
#include <vector>
//------------------------------------------------------------------------------------------------

//***Board Storage***
//------------------------------------------------------------------------------------------------
//Struct of each square on chess board
struct Board_Square
{
	//Occupied by a piece? - True, False
	bool Is_Occupied;
	//Colour of piece - W, B
	char Piece_Colour;
	//Piece Type - R, N, B, K, Q, P
	char Piece_Type;
};

//Initialize board at start of game
void Board_Setup();
//------------------------------------------------------------------------------------------------

//***Move Storage***
//------------------------------------------------------------------------------------------------
//Move struct
struct Move
{
	//Colour making move (thus colour of piece being moved)
	char Colour;
	//Type of piece being moved
	char Piece;
	//Coordinates (row and column) moving between
	int Start_Row, Start_Column, End_Row, End_Column;
	//colour and type of piece captured (no capture = " ")
	char Colour_Captured;
	char Piece_Captured;
};
//------------------------------------------------------------------------------------------------

//***MateBot Search***
//------------------------------------------------------------------------------------------------
//Primary call for searching; returns MateBots top move for given position
Move MateBot_Move(int Search_Depth, char MateBots_Colour);

//NegaMax
//Recursive Negamax function with alpha-beta pruning
//to determine what evaluation each given move leads to
int NegaMax(int Search_Depth, char MateBots_Colour, int alpha, int beta, char Currently_Moving);

//Sort Moves By CV
//personal comparator for sort() function allowing for
//sorting moves by their capture value
bool Sort_Moves_By_CV(Move a, Move b);

//Move Ordering
//Orders moves firstly by capture value, then by value of piece being moved
//vector <Move> Ordered_Moves(vector <Move> Moves);

//Search Depth Optimization
//Considers current move number and number of pieces on board to determine what depth
//MateBot should search to
int Optimal_Search_Depth();

//Search Diagnostics
void Search_Diagnostics(int Search_Time, int Search_Depth, int Terminal_Nodes_Evaluated, string Top_Engine_Move, vector <Move> Ordered_Moves, int Chosen_Move_Eval);
//---------------------------------------------------------------------l
// ---------------------------

//***MateBot Position Evaluation
//------------------------------------------------------------------------------------------------
//MateBots functoin for evaluating a given position
int Position_Evaluation(char Eval_From_Perspective_Of);
//------------------------------------------------------------------------------------------------

//***Move Generation and Making
//------------------------------------------------------------------------------------------------
//Define Move - stores all data for move into a Move struct
Move Define_Move(char Colour, char Piece, int SR, int SC, int ER, int EC, char Colour_Captured, char Piece_Captured);

//Make move function
void Make_Move(Move Move);

//Unmake move functio
void Unmake_Move(Move Move);

//List of possible Moves - calculates and returns all legal moves
//in this position for colour about to move
//Does not take into consideration opponent checks
vector <Move> Possible_Moves(char Colour_To_Move);

//Removes moves which put/leave king in check
vector <Move> Remove_Check_Moves(char Colour_To_Move, vector <Move> Potentially_Legal_Moves);

//Legal Moves for pawn piece
vector <Move> Pawn_Legal_Moves(char Colour_Of_Piece, int row, int column);

//Legal Moves for knight piece
vector <Move> Knight_Legal_Moves(char Colour_Of_Piece, int row, int column);

//Legal Moves for bishop piece
vector <Move> Bishop_Legal_Moves(char Colour_Of_Piece, int row, int column);

//Legal Moves for rook piece
vector <Move> Rook_Legal_Moves(char Colour_Of_Piece, int row, int column);

//Legal Moves for queen piece
vector <Move> Queen_Legal_Moves(char Colour_Of_Piece, int row, int column);

//Legal Moves for knight piece
vector <Move> King_Legal_Moves(char Colour_Of_Piece, int row, int column);
//------------------------------------------------------------------------------------------------

//***Universal Chess Interface Protocol***
//------------------------------------------------------------------------------------------------
//Update internal board state as provided by Lichess
void Update_Board_State(string State_Provided_By_Lichess);

//Convert MateBot's top move to UCI form
string Convert_Move_To_UCI_Form(Move Move);

//Convert UCI form moves to internal storage form
Move Convert_Move_From_UCI_Form(string Move_In_UCI_Form);
//------------------------------------------------------------------------------------------------