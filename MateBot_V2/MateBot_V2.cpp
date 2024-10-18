//(c) MateBot_V2, Chess Engine
//(c) Mate Csapo; August 2023
//Source File

//***PREAMBLE***
//------------------------------------------------------------------------------------------------
//Include iostream  
#include <iostream>
//Include file stream
#include <fstream>
//Include text alignment
#include <iomanip>
//Include time
#include <ctime>
//Include c++ sort() algorithm
#include <algorithm>
//For string parsing
#include <string>
//Vector Library
#include <vector>
//Header File
#include "MateBot_V2.hpp"

//Standard Namespace
using namespace std;
//------------------------------------------------------------------------------------------------

//***Board Storage***
//------------------------------------------------------------------------------------------------
//Define a public array to store chessboard
// - [0][0] is top left square of black's side
Board_Square Board[8][8];

//Initialize board at start of game
void Board_Setup()
{
    //Initialize Black pieces
    //Note top 2 rows as occupied by black
    for (int row = 0; row <= 1; row++)
    {
        for (int column = 0; column <= 7; column++)
        {
            Board[row][column].Is_Occupied = true;
            Board[row][column].Piece_Colour = 'B';
        }
    }
    //Initialize top row
    string Top_Row = "RNBQKBNR";
    for (int column = 0; column <= 7; column++)
    {
        Board[0][column].Piece_Type = Top_Row[column];
    }
    //Initialize second row
    for (int column = 0; column <= 7; column++)
    {
        Board[1][column].Piece_Type = 'P';
    }

    //Initialize empty squares
    for (int row = 2; row <= 5; row++)
    {
        for (int column = 0; column <= 7; column++)
        {
            Board[row][column].Is_Occupied = false;
        }
    }

    //Initialize White pieces
    //Note bottom 2 rows as occupied by white
    for (int row = 6; row <= 7; row++)
    {
        for (int column = 0; column <= 7; column++)
        {
            Board[row][column].Is_Occupied = true;
            Board[row][column].Piece_Colour = 'W';
        }
    }
    //Initialize sixth row
    for (int column = 0; column <= 7; column++)
    {
        Board[6][column].Piece_Type = 'P';
    }
    //Initialize bottom row
    string Bottom_Row = "RNBQKBNR";
    for (int column = 0; column <= 7; column++)
    {
        Board[7][column].Piece_Type = Top_Row[column];
    }
}
//------------------------------------------------------------------------------------------------

//***Colour and Depth Storage***
//------------------------------------------------------------------------------------------------
//Global variable for storing which side MateBot is playing as (for UCI purposes; assumed to be B)
char MateBot_Playing_As = 'B';

//Global variable for storing the current move # of the game; initialized to 0
int Move_Number = 0;

//global variable for storing total number of major pieces left on the board
int Num_Major_Pieces_Left;

//Recent move storage
//storing recent moves to stop bot from playing 3-fold repititions.
vector <Move> Moves_Played;
//------------------------------------------------------------------------------------------------

//***Search Diagnostics Global Variable Storage***
//------------------------------------------------------------------------------------------------
//Temp variable for measuring time
int Total_Time_Temp;
//Quantifyed measurements
int Search_Time, Terminal_Nodes_Evaluated;
//Time profile global vars
int Pseudo_Legal_Time, Legal_Time, Move_Ordering_Time, Pos_Eval_Time;
//Time profile temp var
int Profile_Temp_Time;
//file output var
fstream fout;
//------------------------------------------------------------------------------------------------

// 
//***Piece +/ Positional Value Constants***
//------------------------------------------------------------------------------------------------
//Relative numerical value of each piece type for evaluating
//their worths
int Pawn_Value = 100;
int Knight_Value = 320;
int Bishop_Value = 330;
int Rook_Value = 500;
int Queen_Value = 900;
int King_Value = 2000;
//Note that king has a value of 0

//Piece Square Lists
//White Pieces
int W_Pawn_PSL[64] = { 120, 120, 120, 120, 120, 120, 120, 120,
60, 60, 60, 60, 60, 60, 60, 60,
30, 30, 35, 40, 40, 35, 30, 30,
25, 25, 31, 35, 35, 31, 25, 25,
2, 12, 21, 34, 34, 21, 12, 2,
0, 7, 17, 30, 30, 17, 7, 0,
5, 0, -5, -15, -15, -5, 0, 5,
0, 0, 0, 0, 0, 0, 0, 0 };
int W_Knight_PSL[64] = { -50,-40,-30,-30,-30,-30,-40,-50,-40,-20,  0,  0,  0,  0,-20,-40, -30,  0, 10, 15, 15, 10,  0,-30, -30,  5, 15, 20, 20, 15,  5,-30, -30,  0, 15, 20, 20, 15,  0,-30, -30,  5, 10, 15, 15, 10,  5,-30, -40,-20,  0,  5,  5,  0,-20,-40, -50,-40,-30,-30,-30,-30,-40,-50, };
int W_Bishop_PSL[64] = { -20,-10,-10,-10,-10,-10,-10,-20, -10,  0,  0,  0,  0,  0,  0,-10, -10,  0,  5, 10, 10,  5,  0,-10, -10,  5,  5, 10, 10,  5,  5,-10, -10,  0, 10, 10, 10, 10,  0,-10, -10, 10, 10, 10, 10, 10, 10,-10, -10,  5,  0,  0,  0,  0,  5,-10, -20,-10,-10,-10,-10,-10,-10,-20, };
int W_Rook_PSL[64] = { 0,  0,  0,  0,  0,  0,  0,  0, 5, 10, 10, 10, 10, 10, 10,  5, -5,  0,  0,  0,  0,  0,  0, -5, -5,  0,  0,  0,  0,  0,  0, -5, -5,  0,  0,  0,  0,  0,  0, -5, -5,  0,  0,  0,  0,  0,  0, -5, -5,  0,  0,  0,  0,  0,  0, -5, -15,  -10,  0,  5,  5,  0,  -10,  -15 };
int W_Queen_PSL[64] = { -20,-10,-10, -5, -5,-10,-10,-20, -10,  0,  0,  0,  0,  0,  0,-10, -10,  0,  5,  5,  5,  5,  0,-10, -5,  0,  5,  5,  5,  5,  0, -5, 0,  0,  5,  5,  5,  5,  0, -5, -10,  5,  5,  5,  5,  5,  0,-10, -10,  0,  5,  0,  0,  0,  0,-10, -20,-10,-10, -5, -5,-10,-10,-20 };
int W_King_MiddleGame_PSL[64] = { -30,-40,-40,-50,-50,-40,-40,-30, -30,-40,-40,-50,-50,-40,-40,-30, -30,-40,-40,-50,-50,-40,-40,-30, -30,-40,-40,-50,-50,-40,-40,-30, -20,-30,-30,-40,-40,-30,-30,-20, -10,-20,-20,-20,-20,-20,-20,-10, 20, 20,  0,  0,  0,  0, 20, 20, 20, 30, 10,  0,  0, 10, 30, 20 };
int W_King_EndGame_PSL[64] = { -50,-40,-30,-20,-20,-30,-40,-50, -30,-20,-10,  0,  0,-10,-20,-30, -30,-10, 20, 30, 30, 20,-10,-30, -30,-10, 30, 40, 40, 30,-10,-30, -30,-10, 30, 40, 40, 30,-10,-30, -30,-10, 20, 30, 30, 20,-10,-30, -30,-30,  0,  0,  0,  0,-30,-30, -50,-30,-30,-30,-30,-30,-30,-50 };
//Black pieces
int B_Pawn_PSL[64] = { 0, 0, 0, 0, 0, 0, 0, 0,
5, 0, -5, -15, -15, -5, 0, 5,
0, 7, 17, 30, 30, 17, 7, 0,
2, 12, 21, 34, 34, 21, 12, 2,
25, 25, 31, 35, 35, 31, 25, 25,
30, 30, 35, 40, 40, 35, 30, 30,
60, 60, 60, 60, 60, 60, 60, 60,
80, 80, 80, 80, 80, 80, 80, 80 };;
int B_Knight_PSL[64] = { -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0, 5, 5, 0, -20, -40, -30, 5, 10, 15, 15, 10, 5, -30, -30, 0, 15, 20, 20, 15, 0, -30, -30, 5, 15, 20, 20, 15, 5, -30, -30, 0, 10, 15, 15, 10, 0, -30, -40, -20, 0, 0, 0, 0, -20, -40, -50, -40, -30, -30, -30, -30, -40, -50 };
int B_Bishop_PSL[64] = { -20, -10, -10, -10, -10, -10, -10, -20, -10, 5, 0, 0, 0, 0, 5, -10, -10, 10, 10, 10, 10, 10, 10, -10, -10, 0, 10, 10, 10, 10, 0, -10, -10, 5, 5, 10, 10, 5, 5, -10, -10, 0, 5, 10, 10, 5, 0, -10, -10, 0, 0, 0, 0, 0, 0, -10, -20, -10, -10, -10, -10, -10, -10, -20 };
int B_Rook_PSL[64] = { -15,  -10,  0,  5,  5,  0,  -10,  -15, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, 5, 10, 10, 10, 10, 10, 10, 5, 0, 0, 0, 0, 0, 0, 0, 0, };
int B_Queen_PSL[64] = { -20, -10, -10, -5, -5, -10, -10, -20, -10, 0, 0, 0, 0, 5, 0, -10, -10, 0, 5, 5, 5, 5, 5, -10, -5, 0, 5, 5, 5, 5, 0, 0, -5, 0, 5, 5, 5, 5, 0, -5, -10, 0, 5, 5, 5, 5, 0, -10, -10, 0, 0, 0, 0, 0, 0, -10, -20, -10, -10, -5, -5, -10, -10, -20 };
int B_King_MiddleGame_PSL[64] = { 20, 30, 10, 0, 0, 10, 30, 20, 20, 20, 0, 0, 0, 0, 20, 20, -10, -20, -20, -20, -20, -20, -20, -10, -20, -30, -30, -40, -40, -30, -30, -20, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30 };
int B_King_EndGame_PSL[64] = { -50, -30, -30, -30, -30, -30, -30, -50, -30, -30, 0, 0, 0, 0, -30, -30, -30, -10, 20, 30, 30, 20, -10, -30, -30, -10, 30, 40, 40, 30, -10, -30, -30, -10, 30, 40, 40, 30, -10, -30, -30, -10, 20, 30, 30, 20, -10, -30, -30, -20, -10, 0, 0, -10, -20, -30, -50, -40, -30, -20, -20, -30, -40, -50 };
//------------------------------------------------------------------------------------------------

//***Driver Function***
//------------------------------------------------------------------------------------------------
//Handles UCI interfacing and communication; calls for MateBot search to give top move
int main()
{
    //Open search diagnostics file
    fout.open("Search_Diagnostics.txt");

    //Variable for inputting UCI commands form Lichess
    string Command_Received;
    //Keep playing until Lichess tells us to quit
    while (Command_Received != "quit")
    {
        //Input UCI commands from Lichess
        getline(cin, Command_Received);

        //Response according to what command received was
        if (Command_Received == "uci") cout << "id name MateBot\nid author MVC\nuciok\n";
        else if (Command_Received == "isready") cout << "readyok\n";
        //First chars = pos signifies "position"
        else if ((Command_Received[0] == 'p') and (Command_Received[1] == 'o') and (Command_Received[2] == 's'))
        {
            //update the internal board storage
            Update_Board_State(Command_Received);
            //increment number of moves played by 1
            Move_Number++;
        }
        //go command, telling MateBot to start searching
        else if ((Command_Received[0] == 'g') and (Command_Received[1] == 'o'))
        {
            //Calculate MateBots top move for this position
            Move Top_Move = MateBot_Move(Optimal_Search_Depth(), MateBot_Playing_As);
            //Send Lichess MateBot's top move
            cout << "bestmove " << Convert_Move_To_UCI_Form(Top_Move) << "\n";
        }
    }

    //Close search diagnostics file
    fout.close();
}
//------------------------------------------------------------------------------------------------

//***MateBot Search***
//------------------------------------------------------------------------------------------------
//Primary call for searching; returns MateBots top move for given position
Move MateBot_Move(int Search_Depth, char MateBots_Colour)
{
    //initiliaze temp for search time measurement
    Total_Time_Temp = time(NULL);

    //Zero the profile time measurement variables
    Pseudo_Legal_Time = 0;
    Legal_Time = 0;
    Move_Ordering_Time = 0;
    Pos_Eval_Time = 0;

    //Obtain all potential playable moves for colour_to_move
    vector <Move> Playable_Moves = Possible_Moves(MateBots_Colour);
    Playable_Moves = Remove_Check_Moves(MateBots_Colour, Playable_Moves);

    //Sort moves to optimize pruning
    //Playable_Moves = Ordered_Moves (Playable_Moves);
    sort (Playable_Moves.begin(), Playable_Moves.end(), Sort_Moves_By_CV);

    //Remove any move which would lead to a threefold repition
    for (int i = 0; i < Playable_Moves.size(); i++)
    {
        //would be repition if move from 2 ago and 4 ago equal next move played; make sure past move 6
        if ((Move_Number > 6) and ((Playable_Moves[i].Colour == Moves_Played[Moves_Played.size() - 2].Colour) and (Playable_Moves[i].Piece == Moves_Played[Moves_Played.size() - 2].Piece) and (Playable_Moves[i].Start_Row == Moves_Played[Moves_Played.size() - 2].Start_Row) and (Playable_Moves[i].Start_Column == Moves_Played[Moves_Played.size() - 2].Start_Column) and (Playable_Moves[i].End_Row == Moves_Played[Moves_Played.size() - 2].End_Row) and (Playable_Moves[i].End_Column == Moves_Played[Moves_Played.size() - 2].End_Column)) and ((Playable_Moves[i].Colour == Moves_Played[Moves_Played.size() - 4].Colour) and (Playable_Moves[i].Piece == Moves_Played[Moves_Played.size() - 4].Piece) and (Playable_Moves[i].Start_Row == Moves_Played[Moves_Played.size() - 4].Start_Row) and (Playable_Moves[i].Start_Column == Moves_Played[Moves_Played.size() - 4].Start_Column) and (Playable_Moves[i].End_Row == Moves_Played[Moves_Played.size() - 4].End_Row) and (Playable_Moves[i].End_Column == Moves_Played[Moves_Played.size() - 4].End_Column)))
        {
            //remove the move from the list
            Playable_Moves.erase(Playable_Moves.begin() + i);
        }
    }

    //Variable for storing MateBot's top move; initialized randomly
    Move MateBots_Move = Playable_Moves[0];
    //Variable for storing evaluation of best move; initalized to -infinity
    //so it is easily overcome
    int Best_Eval = -1000000000;

    //Initialize major searfch dianostics variable
    Terminal_Nodes_Evaluated = 0;

    //Minimax all Playable_Moves
    for (int Move_Num = 0; Move_Num < Playable_Moves.size(); Move_Num++)
    {
        //Pseudo make move on M_Board
        Make_Move(Playable_Moves[Move_Num]);
        //Define that opponent is next_to_move
        char Next_To_Move;
        if (MateBots_Colour == 'W') Next_To_Move = 'B';
        else if (MateBots_Colour == 'B') Next_To_Move = 'W';
        //Negamax the move
        int Eval = -NegaMax(Search_Depth, MateBots_Colour, -10000000, 10000000, Next_To_Move);
        //Unmake Move on M_Board (revert M_Board to Temp_Board)
        Unmake_Move(Playable_Moves[Move_Num]);
        //check if this is a better move than found so far
        if (Eval > Best_Eval)
        {
            //update best move and corresponding eval
            MateBots_Move = Playable_Moves[Move_Num];
            Best_Eval = Eval;
        }
    }

    //Output search diagnostics to file
    Search_Diagnostics (time(NULL) - Total_Time_Temp, Search_Depth + 1, Terminal_Nodes_Evaluated, Convert_Move_To_UCI_Form (MateBots_Move), Playable_Moves, Best_Eval);

    //Update Moves_Played Vector
    Moves_Played.push_back (MateBots_Move);
    
    //Return MateBots best move
    return MateBots_Move;
}

//NegaMax
//Recursive Negamax function with alpha-beta pruning
//to determine what evaluation each given move leads to
int NegaMax(int Search_Depth, char MateBots_Colour, int alpha, int beta, char Currently_Moving)
{
    //if this is a terminal node (final position evaluation)
    //return position score playing this move would lead to
    if (Search_Depth == 0)
    {
        //We need to flip the Currently_Moving back
        char Eval_From_Perspective_Of = Currently_Moving;
        //Grab time temp for pos evaluation
        Profile_Temp_Time = time(NULL);
        return Position_Evaluation(Eval_From_Perspective_Of);
        //add pos eval time onto total
        Pos_Eval_Time += time(NULL) - Profile_Temp_Time;
    }

    //Declare variable for storing this branch's best eval
    //initialized very negative so it is easily overcome
    int Branch_Best_Eval = -10000000;

    //Grab time temp for pseudo legal move gen
    Profile_Temp_Time = time(NULL);
    //Obtain all potential playable moves for next_to_move
    vector <Move> Playable_Moves = Possible_Moves(Currently_Moving);
    //Add pseudo legal move gen time onto total
    Pseudo_Legal_Time += time(NULL) - Profile_Temp_Time;
    //Remove illegal moves
    //Grab time temp for legal move gen
    Profile_Temp_Time = time(NULL);
    Playable_Moves = Remove_Check_Moves(Currently_Moving, Playable_Moves);
    //Add legal move gen time onto total
    Legal_Time += time(NULL) - Profile_Temp_Time;


    //Grab time temp for move ordering
    Profile_Temp_Time = time(NULL);
    //Sort moves to optimize pruning
    //Playable_Moves = Ordered_Moves(Playable_Moves);
    sort(Playable_Moves.begin(), Playable_Moves.end(), Sort_Moves_By_CV);
    //add move ordering time onto total
    Move_Ordering_Time += time(NULL) - Profile_Temp_Time;

    //Determine whose player's turn it is to move after this depth
    char Next_To_Move;
    if (Currently_Moving == 'W') Next_To_Move = 'B';
    else if (Currently_Moving == 'B') Next_To_Move = 'W';

    //Call negamax on all moves stemming from this branch
    for (int Move_Num = 0; Move_Num < Playable_Moves.size(); Move_Num++)
    {
        //Pseudo-make move on M_Board
        Make_Move(Playable_Moves[Move_Num]);
        //negamax this move (depth-1) and return evaluation
        int Eval = -NegaMax(Search_Depth - 1, MateBots_Colour, -beta, -alpha, Next_To_Move);
        //Unmake Move on M_Board (revert M_Board to Temp_Board)
        Unmake_Move(Playable_Moves[Move_Num]);
        //if new best eval is found, update Branch_Best_Eval
        if (Eval > Branch_Best_Eval) Branch_Best_Eval = Eval;
        //Alpha-Beta pruning
        alpha = max(alpha, Branch_Best_Eval);
        if (alpha >= beta) break;
    }

    //Return final evaluation resulting from move Negamaxed
    return Branch_Best_Eval;
}

//Sort Moves By CV
//personal comparator for sort() function allowing for
//sorting moves by their capture value
bool Sort_Moves_By_CV(Move a, Move b)
{
    //Move value variables for judging/predicting how good a move is
    int Move_Value_A = 0;
    int Move_Value_B = 0;

    //Calculate capture value for each
    //Calculate capture value of A
    //Calculate piece value moved in a
    int Piece_Value_A = 0;
    if (a.Piece == 'P') Piece_Value_A = Pawn_Value;
    else if (a.Piece == 'N') Piece_Value_A = Knight_Value;
    else if (a.Piece == 'B') Piece_Value_A = Bishop_Value;
    else if (a.Piece == 'Q') Piece_Value_A = Queen_Value;
    else if (a.Piece == 'R') Piece_Value_A = Rook_Value;
    //Calculate value captured by a
    int Value_Captured_A = 0;
    if (a.Piece_Captured == 'P') Value_Captured_A = Pawn_Value;
    else if (a.Piece_Captured == 'N') Value_Captured_A = Knight_Value;
    else if (a.Piece_Captured == 'B') Value_Captured_A = Bishop_Value;
    else if (a.Piece_Captured == 'Q') Value_Captured_A = Queen_Value;
    else if (a.Piece_Captured == 'R') Value_Captured_A = Rook_Value;
    //Calculate capture value of A
    int Capture_Value_A;
    //if not a capturing move, capture value is 0
    if (Value_Captured_A == 0) Capture_Value_A = 0;
    //if capturing move, calculate capture value
    else
    {
        //calculate capture value
        Capture_Value_A = Value_Captured_A - Piece_Value_A;
        //Prioritize this capturing move in move_value
        Move_Value_A += 10000;
    }
    //Add capture value onto move value
    Move_Value_A += Capture_Value_A;

    //Calculate capture value of B
    //Calculate piece value moved in b
    int Piece_Value_B = 0;
    if (b.Piece == 'P') Piece_Value_B = Pawn_Value;
    else if (b.Piece == 'N') Piece_Value_B = Knight_Value;
    else if (b.Piece == 'B') Piece_Value_B = Bishop_Value;
    else if (b.Piece == 'Q') Piece_Value_B = Queen_Value;
    else if (b.Piece == 'R') Piece_Value_B = Rook_Value;
    //Calculate value captured by b
    int Value_Captured_B = 0;
    if (b.Piece_Captured == 'P') Value_Captured_B = Pawn_Value;
    else if (b.Piece_Captured == 'N') Value_Captured_B = Knight_Value;
    else if (b.Piece_Captured == 'B') Value_Captured_B = Bishop_Value;
    else if (b.Piece_Captured == 'Q') Value_Captured_B = Queen_Value;
    else if (b.Piece_Captured == 'R') Value_Captured_B = Rook_Value;
    //Calculate capture value of B
    int Capture_Value_B;
    //if not a capturing move, capture value is 0
    if (Value_Captured_B == 0) Capture_Value_B = 0;
    //if capturing move, calculate capture value
    else
    {
        //calculate capture value
        Capture_Value_B = Value_Captured_B - Piece_Value_B;
        //prioriztize this capturing move in move_value
        Move_Value_B += 10000;
    }
    //Add capture value onto move value
    Move_Value_B += Capture_Value_B;

    //Thanks to the +10000, capturing moves will be prioirizted; next sorting is to sort
    //non-capturing moves by piece value; we can simply sort by adding on their piece_value
    //to the move_value
    if (Value_Captured_A == 0)
    {
        Move_Value_A += Piece_Value_A;
    }
    if (Value_Captured_B == 0)
    {
        Move_Value_B += Piece_Value_B;
    }


    //Return true (sorting algorithm should swap moves)
    //if move value of A is better than B's move value
    return Move_Value_A > Move_Value_B;
}

//Search Depth Optimization
//Considers current move number and number of pieces on board to determine what depth
//MateBot should search to
int Optimal_Search_Depth()
{
    //initialize counters to zero
    Num_Major_Pieces_Left = 0;

    //iterate through whole board
    for (int row = 0; row <= 7; row++)
    {
        for (int column = 0; column <= 7; column++)
        {
            if ((Board[row][column].Piece_Type == 'Q') or (Board[row][column].Piece_Type == 'R') or (Board[row][column].Piece_Type == 'B') or (Board[row][column].Piece_Type == 'N'))
            {
                //add one major piece to counter
                Num_Major_Pieces_Left++;
            }
        }
    }

    //If we have reached endgame, increase the value of pawns
    if (Num_Major_Pieces_Left <= 4) Pawn_Value = 150;

    //For oponenings (first 3 moves), play depth 5
    if (Move_Number <= 3) return 4; // 4 = depth 5
    //if more than 6 major pieces left, look at depth 4
    else if (Num_Major_Pieces_Left > 6) return 3;
    //if five or 6 major pieces left, look at depth 4
    else if ((Num_Major_Pieces_Left == 5) or (Num_Major_Pieces_Left == 6)) return 3;
    //if two, three, or four major pieces left, look at depth 6
    else if ((Num_Major_Pieces_Left == 2) or (Num_Major_Pieces_Left == 3) or (Num_Major_Pieces_Left == 4)) return 5;
    //if one or less major piece left, look at depth 8
    else if (Num_Major_Pieces_Left <= 1) return 7;
}

//Search Diagnostics
void Search_Diagnostics(int Search_Time, int Search_Depth, int Terminal_Nodes_Evaluated, string Top_Engine_Move, vector <Move> Ordered_Moves, int Chosen_Move_Eval)
{
    //Output search diagnostics
    fout << "Search Diagnostics for move " << Move_Number << ": " << endl;
    fout << "Move Chosen --> " << Top_Engine_Move << endl;
    fout << "Chosen_Move_Eval --> " << Chosen_Move_Eval << endl;
    fout << "Search Depth --> " << Search_Depth << endl;
    fout << "Total Terminal Nodes Evaluated --> " << Terminal_Nodes_Evaluated << endl;
    fout << "Total Search Time --> " << Search_Time << endl;
    fout << "Search Time Profile:" << endl;
    fout << "  - Total Pseudo-Legal Move Gen Time --> " << Pseudo_Legal_Time << endl;
    fout << "  - Total Legal Move Gen Time --> " << Legal_Time << endl;
    fout << "  - Total Move Ordering Time --> " << Move_Ordering_Time << endl;
    fout << "  - Total Position Evaluation Time --> " << Pos_Eval_Time << endl;
    fout << "Moves Ordered From Best To Worst: ";
    for (int i = 0; i < Ordered_Moves.size(); i++)
    {
        fout << Convert_Move_To_UCI_Form(Ordered_Moves[i]) << " ";
    }
    fout << endl;
    fout << endl;
}
//------------------------------------------------------------------------------------------------

//***MateBot Position Evaluation
//------------------------------------------------------------------------------------------------
//MateBots functoin for evaluating a given position
int Position_Evaluation(char Eval_From_Perspective_Of)
{
    //Update total number of nodes evaluated
    Terminal_Nodes_Evaluated++;

    //Determine a numerical value representing which player black vs. white is better off
    //in this given board position
    //This eval currently considers two things:
    //1. pure material value of both sides
    //2. positional value of both sides pieces
    int Eval_Of_Position = 0; //+ = advantage for Eval_From_Perspective_Of
    //- = advantage for other side

    //Variables for storing each side's total value
    int Eval_From_Perspective_Ofs_Value = 0;
    int Opponents_Value = 0;

    //Go through whole board
    for (int row = 0; row <= 7; row++)
    {
        for (int column = 0; column <= 7; column++)
        {
            //If square has no piece, skip it as it does not contribure
            //do either sides evaluation
            if (Board[row][column].Is_Occupied == false) continue;

            //else occupied by a piece, calculate the total value of each square
            int Square_Value = 0;

            //Calculate pure value of piece
            if (Board[row][column].Piece_Type == 'P') Square_Value += Pawn_Value;
            else if (Board[row][column].Piece_Type == 'N') Square_Value += Knight_Value;
            else if (Board[row][column].Piece_Type == 'B') Square_Value += Bishop_Value;
            else if (Board[row][column].Piece_Type == 'R') Square_Value += Rook_Value;
            else if (Board[row][column].Piece_Type == 'Q') Square_Value += Queen_Value;
            else if (Board[row][column].Piece_Type == 'K') Square_Value += King_Value;

            //Calculate PSL bonus for position
            //white pawn regular
            if ((Board[row][column].Piece_Colour == 'W') and (Board[row][column].Piece_Type == 'P')) Square_Value += W_Pawn_PSL[(row * 8) + (column)];
            else if ((Board[row][column].Piece_Colour == 'W') and (Board[row][column].Piece_Type == 'N')) Square_Value += W_Knight_PSL[(row * 8) + (column)];
            else if ((Board[row][column].Piece_Colour == 'W') and (Board[row][column].Piece_Type == 'B')) Square_Value += W_Bishop_PSL[(row * 8) + (column)];
            else if ((Board[row][column].Piece_Colour == 'W') and (Board[row][column].Piece_Type == 'R')) Square_Value += W_Rook_PSL[(row * 8) + (column)];
            else if ((Board[row][column].Piece_Colour == 'W') and (Board[row][column].Piece_Type == 'Q')) Square_Value += W_Queen_PSL[(row * 8) + (column)];
            else if ((Board[row][column].Piece_Colour == 'W') and (Board[row][column].Piece_Type == 'K')) Square_Value += W_King_MiddleGame_PSL[(row * 8) + (column)];
            else if ((Board[row][column].Piece_Colour == 'B') and (Board[row][column].Piece_Type == 'P')) Square_Value += B_Pawn_PSL[(row * 8) + (column)];
            else if ((Board[row][column].Piece_Colour == 'B') and (Board[row][column].Piece_Type == 'N')) Square_Value += B_Knight_PSL[(row * 8) + (column)];
            else if ((Board[row][column].Piece_Colour == 'B') and (Board[row][column].Piece_Type == 'B')) Square_Value += B_Bishop_PSL[(row * 8) + (column)];
            else if ((Board[row][column].Piece_Colour == 'B') and (Board[row][column].Piece_Type == 'R')) Square_Value += B_Rook_PSL[(row * 8) + (column)];
            else if ((Board[row][column].Piece_Colour == 'B') and (Board[row][column].Piece_Type == 'Q')) Square_Value += B_Queen_PSL[(row * 8) + (column)];
            else if ((Board[row][column].Piece_Colour == 'B') and (Board[row][column].Piece_Type == 'K')) Square_Value += B_King_MiddleGame_PSL[(row * 8) + (column)];

            //Give a bonus for pawns close to promotion during endgame
            if ((Board[row][column].Piece_Type == 'P') and (Num_Major_Pieces_Left <= 4))
            {
                //for white pawns
                if (Board[row][column].Piece_Colour == 'W')
                {
                    //the closer they are to promotion, the more the pawns are worth
                    if (row == 3) Square_Value += 50;
                    else if (row == 2) Square_Value += 100;
                    else if (row == 1) Square_Value += 150;
                    else if (row == 0) Square_Value += 200;
                }
                //for black pieces
                else if (Board[row][column].Piece_Colour == 'B')
                {
                    //the closer they are to promotion, the more the pawns are worth
                    if (row == 4) Square_Value += 50;
                    else if (row == 5) Square_Value += 100;
                    else if (row == 6) Square_Value += 150;
                    else if (row == 7) Square_Value += 200;
                }
            }

            //add square value to its side's total
            //if square value is one of Eval_From_Perspective_Of's pieces, add its value to its total
            if (Board[row][column].Piece_Colour == Eval_From_Perspective_Of) Eval_From_Perspective_Ofs_Value += Square_Value;
            //if it is one of the opponent's pieces, add its value to opponents total
            else Opponents_Value += Square_Value;
        }
    }

    //Return position evaluation (recall + = advantage for Eval_From_Position_Of
    Eval_Of_Position = Eval_From_Perspective_Ofs_Value - Opponents_Value;
    return Eval_Of_Position;
}
//------------------------------------------------------------------------------------------------

//***Move Generation and Making
//------------------------------------------------------------------------------------------------
//Define Move - stores all data for move into a Move struct
Move Define_Move(char Colour, char Piece, int SR, int SC, int ER, int EC, char Colour_Captured, char Piece_Captured)
{
    //Define a move struct to return
    Move Permissible_Move;
    //define the move
    Permissible_Move.Colour = Colour;
    Permissible_Move.Piece = Piece;
    Permissible_Move.Start_Row = SR;
    Permissible_Move.Start_Column = SC;
    Permissible_Move.End_Row = ER;
    Permissible_Move.End_Column = EC;
    Permissible_Move.Colour_Captured = Colour_Captured;
    Permissible_Move.Piece_Captured = Piece_Captured;

    //return the move as a struct
    return Permissible_Move;
}

//Make move function
void Make_Move(Move Move)
{
    //we must consider the special case of en passant
    //the regular encoded move is played, but the captured pawn must also be removed
    //detect en passant by being a diagonal move by a pawn to an unnocupied square
    if ((Move.Piece == 'P') and (Move.End_Column != Move.Start_Column) and (Board[Move.End_Row][Move.End_Column].Is_Occupied == false))
    {
        //the regular move of the capturing pawn will be played later in this function
        //we must simply erase the captured pawn from the internal board storage
        //the captured pawn is at Move.Start_Row and Move.End_column
        //erase the captured pawn
        Board[Move.Start_Row][Move.End_Column].Is_Occupied = false;
        Board[Move.Start_Row][Move.End_Column].Piece_Colour = ' ';
        Board[Move.Start_Row][Move.End_Column].Piece_Type = ' ';
    }

    //update the global board
    //remove piece from original position
    Board[Move.Start_Row][Move.Start_Column].Is_Occupied = false;
    Board[Move.Start_Row][Move.Start_Column].Piece_Colour = ' ';
    Board[Move.Start_Row][Move.Start_Column].Piece_Type = ' ';
    //overwrite square it piece is moving to with new piece
    Board[Move.End_Row][Move.End_Column].Is_Occupied = true;
    Board[Move.End_Row][Move.End_Column].Piece_Colour = Move.Colour;
    Board[Move.End_Row][Move.End_Column].Piece_Type = Move.Piece;

    //we must detect the special case of promotion
    //we will assume pawns always promote to a queen
    //promotion can be detected by a pawn moving to row 0 or 7
    if ((Move.Piece == 'P') and ((Move.End_Row == 0) or (Move.End_Row == 7)))
    {
        //the move has already been made above in the function, but we must
        //change the pawn to a queen
        Board[Move.End_Row][Move.End_Column].Piece_Type = 'Q';
    }
}

//Unmake move functio
void Unmake_Move(Move Move)
{
    //update the global board
    //Move piece back to original position
    Board[Move.Start_Row][Move.Start_Column].Is_Occupied = true;
    Board[Move.Start_Row][Move.Start_Column].Piece_Colour = Move.Colour;
    Board[Move.Start_Row][Move.Start_Column].Piece_Type = Move.Piece;
    //Reset square that was moved to (including adding back any captured pieces)
    //Is_Occupied depends on if move was a capture
    //for capturing moves
    if ((Move.Colour_Captured == 'W') or (Move.Colour_Captured == 'B'))
    {
        Board[Move.End_Row][Move.End_Column].Is_Occupied = true;
    }
    //otherwise, if non-capturing move
    else Board[Move.End_Row][Move.End_Column].Is_Occupied = false;
    //reset colour and piece type on end square
    Board[Move.End_Row][Move.End_Column].Piece_Colour = Move.Colour_Captured;
    Board[Move.End_Row][Move.End_Column].Piece_Type = Move.Piece_Captured;
}

//List of possible Moves - calculates and returns all legal moves
//in this position for colour about to move
//Does not take into consideration opponent checks
vector <Move> Possible_Moves(char Colour_To_Move)
{
    //Vector for housing all legal moves
    vector <Move> Legal_Moves;
    //Identify all legal moves possible with colour_to_move pieces
    for (int row = 0; row <= 7; row++)
    {
        for (int column = 0; column <= 7; column++)
        {
            //check if piece on this square is of colour_to_move
            if (Board[row][column].Piece_Colour == Colour_To_Move)
            {
                //Determine and Store all legal moves of this piece
                vector <Move> Additional_Legal_Moves;
                //if piece is a queen
                if (Board[row][column].Piece_Type == 'Q')
                {
                    Additional_Legal_Moves = Queen_Legal_Moves(Colour_To_Move, row, column);
                }
                //if piece is a rook
                else if (Board[row][column].Piece_Type == 'R')
                {
                    Additional_Legal_Moves = Rook_Legal_Moves(Colour_To_Move, row, column);
                }
                //if piece is a bishop
                else if (Board[row][column].Piece_Type == 'B')
                {
                    Additional_Legal_Moves = Bishop_Legal_Moves(Colour_To_Move, row, column);
                }
                //if piece is a pawn
                else if (Board[row][column].Piece_Type == 'P')
                {
                    Additional_Legal_Moves = Pawn_Legal_Moves(Colour_To_Move, row, column);
                }
                //if piece is a knight
                else if (Board[row][column].Piece_Type == 'N')
                {
                    Additional_Legal_Moves = Knight_Legal_Moves(Colour_To_Move, row, column);
                }
                //if piece is a king
                else if (Board[row][column].Piece_Type == 'K')
                {
                    Additional_Legal_Moves = King_Legal_Moves(Colour_To_Move, row, column);
                }
                //Add this pieces legal moves to list
                Legal_Moves.insert(Legal_Moves.end(), Additional_Legal_Moves.begin(), Additional_Legal_Moves.end());
            }
        }
    }
    //Return all Legal Moves
    return Legal_Moves;
}

//Removes moves which put/leave king in check
vector <Move> Remove_Check_Moves(char King_Colour, vector <Move> Potentially_Legal_Moves)
{
    //Define a vector for storing fully legal moves (do not leave/put king in check)
    vector <Move> Fully_Legal_Moves;

    //Determine colour of opponent
    char Opponent_Colour;
    if (King_Colour == 'W') Opponent_Colour = 'B';
    else if (King_Colour == 'B') Opponent_Colour = 'W';

    //Test every single potentially legal move, seeing if it leaves/puts king in check
    for (int i = 0; i < Potentially_Legal_Moves.size(); i++)
    {
        //pseudo make the move
        Make_Move(Potentially_Legal_Moves[i]);

        //Determine coordinates of Colour_To_Move king
        int King_Row = 0;
        int King_Column = 0;
        for (int r = 0; r <= 7; r++)
        {
            for (int c = 0; c <= 7; c++)
            {
                //this this square has colour_to_move's king
                if ((Board[r][c].Piece_Colour == King_Colour) and (Board[r][c].Piece_Type == 'K'))
                {
                    //note r, c as the friendly king's coordinates
                    King_Row = r;
                    King_Column = c;
                }
            }
        }

        //now check if king who made move is in check by seeing if it
        //has a clear line of sight to an opponent
        bool Causes_Check = false;

        //Determine if there is a check delivered from upwards
        for (int New_Row = King_Row - 1; New_Row >= 0; New_Row--)
        {
            //if square is empty, do nothing
            if (Board[New_Row][King_Column].Is_Occupied == false) continue;
            //if square is occupied by a friendly piece
            else if (Board[New_Row][King_Column].Piece_Colour == King_Colour)
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a non-check causing enemy piece
            else if ((Board[New_Row][King_Column].Piece_Type == 'P') or (Board[New_Row][King_Column].Piece_Type == 'N') or (Board[New_Row][King_Column].Piece_Type == 'B') or (Board[New_Row][King_Column].Piece_Type == 'K'))
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a check causing enemy piece
            else if ((Board[New_Row][King_Column].Piece_Type == 'Q') or (Board[New_Row][King_Column].Piece_Type == 'R'))
            {
                //this piece causes check
                Causes_Check = true;
                //We can stop searching further for checks because we already
                //know that this potentially legal move induces check
                break;
            }
        }

        //Determine if there is a check delivered from downwards
        for (int New_Row = King_Row + 1; New_Row <= 7; New_Row++)
        {
            //if square is empty, do nothing
            if (Board[New_Row][King_Column].Is_Occupied == false) continue;
            //if square is occupied by a friendly piece
            else if (Board[New_Row][King_Column].Piece_Colour == King_Colour)
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a non-check causing enemy piece
            else if ((Board[New_Row][King_Column].Piece_Type == 'P') or (Board[New_Row][King_Column].Piece_Type == 'N') or (Board[New_Row][King_Column].Piece_Type == 'B') or (Board[New_Row][King_Column].Piece_Type == 'K'))
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a check causing enemy piece
            else if ((Board[New_Row][King_Column].Piece_Type == 'Q') or (Board[New_Row][King_Column].Piece_Type == 'R'))
            {
                //this piece causes check
                Causes_Check = true;
                //We can stop searching further for checks because we already
                //know that this potentially legal move induces check
                break;
            }
        }

        //Determine if there is a check delivered from the left
        for (int New_Column = King_Column - 1; New_Column >= 0; New_Column--)
        {
            //if square is empty, do nothing
            if (Board[King_Row][New_Column].Is_Occupied == false) continue;
            //if square is occupied by a friendly piece
            else if (Board[King_Row][New_Column].Piece_Colour == King_Colour)
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a non-check causing enemy piece
            else if ((Board[King_Row][New_Column].Piece_Type == 'P') or (Board[King_Row][New_Column].Piece_Type == 'N') or (Board[King_Row][New_Column].Piece_Type == 'B') or (Board[King_Row][New_Column].Piece_Type == 'K'))
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a check causing enemy piece
            else if ((Board[King_Row][New_Column].Piece_Type == 'Q') or (Board[King_Row][New_Column].Piece_Type == 'R'))
            {
                //this piece causes check
                Causes_Check = true;
                //We can stop searching further for checks because we already
                //know that this potentially legal move induces check
                break;
            }
        }

        //Determine if there is a check delivered from the right
        for (int New_Column = King_Column + 1; New_Column <= 7; New_Column++)
        {
            //if square is empty, do nothing
            if (Board[King_Row][New_Column].Is_Occupied == false) continue;
            //if square is occupied by a friendly piece
            else if (Board[King_Row][New_Column].Piece_Colour == King_Colour)
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a non-check causing enemy piece
            else if ((Board[King_Row][New_Column].Piece_Type == 'P') or (Board[King_Row][New_Column].Piece_Type == 'N') or (Board[King_Row][New_Column].Piece_Type == 'B') or (Board[King_Row][New_Column].Piece_Type == 'K'))
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a check causing enemy piece
            else if ((Board[King_Row][New_Column].Piece_Type == 'Q') or (Board[King_Row][New_Column].Piece_Type == 'R'))
            {
                //this piece causes check
                Causes_Check = true;
                //We can stop searching further for checks because we already
                //know that this potentially legal move induces check
                break;
            }
        }

        //Determine if there is a check delivered from the left up diagonal
        for (int New_Row = King_Row - 1, New_Column = King_Column - 1; New_Row >= 0 and New_Column >= 0; New_Row--, New_Column--)
        {
            //if square is empty, do nothing
            if (Board[New_Row][New_Column].Is_Occupied == false) continue;
            //if square is occupied by a friendly piece
            else if (Board[New_Row][New_Column].Piece_Colour == King_Colour)
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a non-check causing enemy piece
            else if ((Board[New_Row][New_Column].Piece_Type == 'P') or (Board[New_Row][New_Column].Piece_Type == 'N') or (Board[New_Row][New_Column].Piece_Type == 'R') or (Board[New_Row][New_Column].Piece_Type == 'K'))
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a check causing enemy piece
            else if ((Board[New_Row][New_Column].Piece_Type == 'Q') or (Board[New_Row][New_Column].Piece_Type == 'B'))
            {
                //this piece causes check
                Causes_Check = true;
                //We can stop searching further for checks because we already
                //know that this potentially legal move induces check
                break;
            }
        }

        //Determine if there is a check delivered from the right up diagonal
        for (int New_Row = King_Row - 1, New_Column = King_Column + 1; New_Row >= 0 and New_Column <= 7; New_Row--, New_Column++)
        {
            //if square is empty, do nothing
            if (Board[New_Row][New_Column].Is_Occupied == false) continue;
            //if square is occupied by a friendly piece
            else if (Board[New_Row][New_Column].Piece_Colour == King_Colour)
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a non-check causing enemy piece
            else if ((Board[New_Row][New_Column].Piece_Type == 'P') or (Board[New_Row][New_Column].Piece_Type == 'N') or (Board[New_Row][New_Column].Piece_Type == 'R') or (Board[New_Row][New_Column].Piece_Type == 'K'))
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a check causing enemy piece
            else if ((Board[New_Row][New_Column].Piece_Type == 'Q') or (Board[New_Row][New_Column].Piece_Type == 'B'))
            {
                //this piece causes check
                Causes_Check = true;
                //We can stop searching further for checks because we already
                //know that this potentially legal move induces check
                break;
            }
        }

        //Determine if there is a check delivered from the left down diagonal
        for (int New_Row = King_Row + 1, New_Column = King_Column - 1; New_Row <= 7 and New_Column >= 0; New_Row++, New_Column--)
        {
            //if square is empty, do nothing
            if (Board[New_Row][New_Column].Is_Occupied == false) continue;
            //if square is occupied by a friendly piece
            else if (Board[New_Row][New_Column].Piece_Colour == King_Colour)
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a non-check causing enemy piece
            else if ((Board[New_Row][New_Column].Piece_Type == 'P') or (Board[New_Row][New_Column].Piece_Type == 'N') or (Board[New_Row][New_Column].Piece_Type == 'R') or (Board[New_Row][New_Column].Piece_Type == 'K'))
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a check causing enemy piece
            else if ((Board[New_Row][New_Column].Piece_Type == 'Q') or (Board[New_Row][New_Column].Piece_Type == 'B'))
            {
                //this piece causes check
                Causes_Check = true;
                //We can stop searching further for checks because we already
                //know that this potentially legal move induces check
                break;
            }
        }

        //Determine if there is a check delivered from the right down diagonal
        for (int New_Row = King_Row + 1, New_Column = King_Column + 1; New_Row <= 7 and New_Column <= 7; New_Row++, New_Column++)
        {
            //if square is empty, do nothing
            if (Board[New_Row][New_Column].Is_Occupied == false) continue;
            //if square is occupied by a friendly piece
            else if (Board[New_Row][New_Column].Piece_Colour == King_Colour)
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a non-check causing enemy piece
            else if ((Board[New_Row][New_Column].Piece_Type == 'P') or (Board[New_Row][New_Column].Piece_Type == 'N') or (Board[New_Row][New_Column].Piece_Type == 'R') or (Board[New_Row][New_Column].Piece_Type == 'K'))
            {
                //this piece blocks any potential check coming from this direction
                //this direction thus does not cause check
                //we can stop searching for checks in this direciton
                break;
            }
            //if square is occupied by a check causing enemy piece
            else if ((Board[New_Row][New_Column].Piece_Type == 'Q') or (Board[New_Row][New_Column].Piece_Type == 'B'))
            {
                //this piece causes check
                Causes_Check = true;
                //We can stop searching further for checks because we already
                //know that this potentially legal move induces check
                break;
            }
        }

        //Determine if there is a check delivered from a Knight/Horse
        //Determine all squares from which a horse could land onto king's square
        //such departure squares are +-2 row/column and +-1 row/column
        for (int r = 0; r <= 7; r++)
        {
            for (int c = 0; c <= 7; c++)
            {
                //check if a suitable departure square
                if ((((r == King_Row + 2) or (r == King_Row - 2)) and ((c == King_Column + 1) or (c == King_Column - 1))) or (((c == King_Column + 2) or (c == King_Column - 2)) and ((r == King_Row + 1) or (r == King_Row - 1))))
                {
                    //check if there is an opponent horse on this square
                    //if yes, this gives check
                    if ((Board[r][c].Piece_Colour == Opponent_Colour) and (Board[r][c].Piece_Type == 'N'))
                    {
                        Causes_Check = true;
                    }
                    //if there is no opponent horse there, do nothing
                }
            }
        }

        //Determine if there is a check delivered from an opponent pawn
        //since pawns can only deliver check from one direction, this must
        //be handled separately for black/white friendly kings
        //in case of friendly king = white
        if (King_Colour == 'W')
        {
            //search squares immediately to top left and top right of king
            //if opponent pawns are on those squares, they cause check
            //top left
            if ((Board[King_Row - 1][King_Column - 1].Piece_Colour == Opponent_Colour) and (Board[King_Row - 1][King_Column - 1].Piece_Type == 'P'))
            {
                Causes_Check = true;
            }
            //top right
            if ((Board[King_Row - 1][King_Column + 1].Piece_Colour == Opponent_Colour) and (Board[King_Row - 1][King_Column + 1].Piece_Type == 'P'))
            {
                Causes_Check = true;
            }
        }
        //in case of friendly king = black
        else if (King_Colour == 'B')
        {
            //search squares immediately to bottom left and bottom right of king
            //if opponent pawns are on those squares, they cause check
            //bottom left
            if ((Board[King_Row + 1][King_Column - 1].Piece_Colour == Opponent_Colour) and (Board[King_Row + 1][King_Column - 1].Piece_Type == 'P'))
            {
                Causes_Check = true;
            }
            //bottom right
            if ((Board[King_Row + 1][King_Column + 1].Piece_Colour == Opponent_Colour) and (Board[King_Row + 1][King_Column + 1].Piece_Type == 'P'))
            {
                Causes_Check = true;
            }
        }

        //undo the move we made
        Unmake_Move(Potentially_Legal_Moves[i]);

        //include the move (append to fully legal vector) we just checked if it does not cause check
        //(causes_check = false)
        if (Causes_Check == false) Fully_Legal_Moves.insert(Fully_Legal_Moves.end(), Potentially_Legal_Moves[i]);
    }
    //Return the fully legal moves
    return Fully_Legal_Moves;
}

//Legal Moves for pawn piece
vector <Move> Pawn_Legal_Moves(char Colour_Of_Piece, int row, int column)
{
    //Vector of moves for storing this pieces legal moves
    vector <Move> Legal_Moves;

    //pawn can make 3 types of legal moves
    //1. one space forward
    //2. two space forward if on second last row
    //3. capture to the diagonally
    //4. en passant (I will deal with this later)
    //white and black pawn moves must be considered separately

    //For white pawn pieces
    if (Colour_Of_Piece == 'W')
    {
        //check for 1. - regular one space forward move - This move is legal if
        //space is unnocpied or occupied by black
        //white is on bottom of board
        //if space is unnocupied
        if (Board[row - 1][column].Is_Occupied == false)
        {
            //define move since it is legal
            Move New_Move = Define_Move(Colour_Of_Piece, 'P', row, column, row - 1, column, Board[row - 1][column].Piece_Colour, Board[row - 1][column].Piece_Type);
            //add it as a legal move
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //if on second last row, consider if double move forward is legal
            if ((row == 6) and (Board[row - 2][column].Is_Occupied == false))
            {
                //define move since it is legal
                Move New_Move = Define_Move(Colour_Of_Piece, 'P', row, column, row - 2, column, Board[row - 2][column].Piece_Colour, Board[row - 2][column].Piece_Type);
                //add it as a legal move
                Legal_Moves.insert(Legal_Moves.end(), New_Move);
            }
        }
        //Check for 3. diagonal capture is legal if occupied by piece of other colour
        //diagonal leftup as long as not on column = 0
        if ((column != 0) and (Board[row - 1][column - 1].Piece_Colour == 'B'))
        {
            //define move since it is legal
            Move New_Move = Define_Move(Colour_Of_Piece, 'P', row, column, row - 1, column - 1, Board[row - 1][column - 1].Piece_Colour, Board[row - 1][column - 1].Piece_Type);
            //add it as a legal move
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //diagonal right up as long as not on column = 7
        if ((column != 7) and (Board[row - 1][column + 1].Piece_Colour == 'B'))
        {
            //define move since it is legal
            Move New_Move = Define_Move(Colour_Of_Piece, 'P', row, column, row - 1, column + 1, Board[row - 1][column + 1].Piece_Colour, Board[row - 1][column + 1].Piece_Type);
            //add it as a legal move
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
    }

    //For black pawn pieces
    else
    {
        //check for 1. - regular one space forward move - This move is legal if
        //space is unnocpied or occupied by white
        //black is on top of board
        //if space is unnocupied
        if (Board[row + 1][column].Is_Occupied == false)
        {
            //define move since it is legal
            Move New_Move = Define_Move(Colour_Of_Piece, 'P', row, column, row + 1, column, Board[row + 1][column].Piece_Colour, Board[row + 1][column].Piece_Type);
            //add it as a legal move
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //if on second last row, consider if double move forward is legal
            if ((row == 1) and (Board[row + 2][column].Is_Occupied == false))
            {
                //define move since it is legal
                Move New_Move = Define_Move(Colour_Of_Piece, 'P', row, column, row + 2, column, Board[row + 2][column].Piece_Colour, Board[row + 2][column].Piece_Type);
                //add it as a legal move
                Legal_Moves.insert(Legal_Moves.end(), New_Move);
            }
        }
        //Ckheck for 3. diagonal capture is legal if occupied by piece of other colour
        //diagonal left down as long as not on column = 0
        if ((column != 0) and (Board[row + 1][column - 1].Piece_Colour == 'W'))
        {
            //define move since it is legal
            Move New_Move = Define_Move(Colour_Of_Piece, 'P', row, column, row + 1, column - 1, Board[row + 1][column - 1].Piece_Colour, Board[row + 1][column - 1].Piece_Type);
            //add it as a legal move
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //diagonal right down as long as not column = 7
        if ((column != 7) and (Board[row + 1][column + 1].Piece_Colour == 'W'))
        {
            //define move since it is legal
            Move New_Move = Define_Move(Colour_Of_Piece, 'P', row, column, row + 1, column + 1, Board[row + 1][column + 1].Piece_Colour, Board[row + 1][column + 1].Piece_Type);
            //add it as a legal move
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
    }

    //Return all legal pawn moves
    return Legal_Moves;
}

//Legal Moves for knight piece
vector <Move> Knight_Legal_Moves(char Colour_Of_Piece, int row, int column)
{
    //Vector of moves for storing this pieces legal moves
    vector <Move> Legal_Moves;

    //strategy for knight legal moves
    //1. identify all possible square to which knight could 'L' move
    //2. check if that square is a legal move

    //identify 'L' movable squares
    //rifle through all squares
    for (int r = 0; r <= 7; r++)
    {
        for (int c = 0; c <= 7; c++)
        {
            //check if an 'L' movable square
            //'L' movable if +-2 row/column and +-1 row/column
            if ((((r == row + 2) or (r == row - 2)) and ((c == column + 1) or (c == column - 1))) or (((c == column + 2) or (c == column - 2)) and ((r == row + 1) or (r == row - 1))))
            {
                //for 'L' movable squares, check if they are a legal move
                if ((Board[r][c].Is_Occupied == false) or (Board[r][c].Piece_Colour != Colour_Of_Piece))
                {
                    //define move since it is legal
                    Move New_Move = Define_Move(Colour_Of_Piece, 'N', row, column, r, c, Board[r][c].Piece_Colour, Board[r][c].Piece_Type);
                    //add it as a legal move
                    Legal_Moves.insert(Legal_Moves.end(), New_Move);
                }
            }
        }
    }

    //Return all legal pawn moves
    return Legal_Moves;
}

//Legal Moves for bishop piece
vector <Move> Bishop_Legal_Moves(char Colour_Of_Piece, int row, int column)
{
    //Vector of moves for storing this pieces legal moves
    vector <Move> Legal_Moves;

    //Identify legal left up diagonal moves
    //Bishop can legally move left up diagonally until encountering other piece
    //Consider all moves left up diagonal till end of board
    for (int New_Row = row - 1, New_Column = column - 1; (New_Row >= 0) and (New_Column >= 0); New_Row--, New_Column--)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'B', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'B', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching left up diagonal
            //; bishop can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves left up diagonal
            break;
        }
    }

    //Identify legal right up diagonal moves
    //Bishop can legally move right up diagonally until encountering other piece
    //Consider all moves right up diagonal till end of board
    for (int New_Row = row - 1, New_Column = column + 1; New_Row >= 0 and New_Column <= 7; New_Row--, New_Column++)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'B', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'B', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching right up diagonal
            //; bishop can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves right up diagonal
            break;
        }
    }

    //Identify legal left down diagonal moves
    //bishop can legally move left down diagonally until encountering other piece
    //Consider all moves left down diagonal till end of board
    for (int New_Row = row + 1, New_Column = column - 1; New_Row <= 7 and New_Column >= 0; New_Row++, New_Column--)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'B', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'B', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching left down diagonal
            //; bishop can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves left down diagonal
            break;
        }
    }

    //Identify legal right down diagonal moves
    //bishop can legally move right down diagonally until encountering 
    //other piece
    //Consider all moves rigth down diagonal till end of board
    for (int New_Row = row + 1, New_Column = column + 1; New_Row <= 7 and New_Column <= 7; New_Row++, New_Column++)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'B', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'B', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching right down diagonal
            //; bishop can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves right down diagonal
            break;
        }
    }

    //Return all legal moves for this bishop piece
    return Legal_Moves;
}

//Legal Moves for rook piece
vector <Move> Rook_Legal_Moves(char Colour_Of_Piece, int row, int column)
{
    //Vector of moves for storing this pieces legal moves
    vector <Move> Legal_Moves;

    //Identify legal upward moves
    //Rook can legally move upward until encountering other piece
    //Consider all moves upward till end of board
    for (int New_Row = row - 1; New_Row >= 0; New_Row--)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'R', row, column, New_Row, column, Board[New_Row][column].Piece_Colour, Board[New_Row][column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'R', row, column, New_Row, column, Board[New_Row][column].Piece_Colour, Board[New_Row][column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching upwards; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves upwards
            break;
        }
    }

    //Identify legal downward moves
    //Rook can legally move downward until encountering other piece
    //Consider all moves downward till end of board
    for (int New_Row = row + 1; New_Row <= 7; New_Row++)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'R', row, column, New_Row, column, Board[New_Row][column].Piece_Colour, Board[New_Row][column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'R', row, column, New_Row, column, Board[New_Row][column].Piece_Colour, Board[New_Row][column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching downwards; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves downwards
            break;
        }
    }

    //Identify legal leftward moves
    //Rook can legally move left until encountering other piece
    //Consider all moves left till end of board
    for (int New_Column = column - 1; New_Column >= 0; New_Column--)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'R', row, column, row, New_Column, Board[row][New_Column].Piece_Colour, Board[row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'R', row, column, row, New_Column, Board[row][New_Column].Piece_Colour, Board[row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching left; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves left
            break;
        }
    }

    //Identify legal rightward moves
    //Rook can legally move right until encountering other piece
    //Consider all moves right till end of board
    for (int New_Column = column + 1; New_Column <= 7; New_Column++)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'R', row, column, row, New_Column, Board[row][New_Column].Piece_Colour, Board[row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'R', row, column, row, New_Column, Board[row][New_Column].Piece_Colour, Board[row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching right; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves right
            break;
        }
    }

    //Return all legal moves for this rook piece
    return Legal_Moves;
}

//Legal Moves for queen piece
vector <Move> Queen_Legal_Moves(char Colour_Of_Piece, int row, int column)
{
    //Vector of moves for storing this pieces legal moves
    vector <Move> Legal_Moves;

    //Identify legal upward moves
    //Queen can legally move upward until encountering other piece
    //Consider all moves upward till end of board
    for (int New_Row = row - 1; New_Row >= 0; New_Row--)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, column, Board[New_Row][column].Piece_Colour, Board[New_Row][column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, column, Board[New_Row][column].Piece_Colour, Board[New_Row][column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching upwards; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves upwards
            break;
        }
    }

    //Identify legal downward moves
    //Queen can legally move downward until encountering other piece
    //Consider all moves downward till end of board
    for (int New_Row = row + 1; New_Row <= 7; New_Row++)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, column, Board[New_Row][column].Piece_Colour, Board[New_Row][column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, column, Board[New_Row][column].Piece_Colour, Board[New_Row][column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching downwards; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves downwards
            break;
        }
    }

    //Identify legal leftward moves
    //Queen can legally move left until encountering other piece
    //Consider all moves left till end of board
    for (int New_Column = column - 1; New_Column >= 0; New_Column--)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, row, New_Column, Board[row][New_Column].Piece_Colour, Board[row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, row, New_Column, Board[row][New_Column].Piece_Colour, Board[row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching left; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves left
            break;
        }
    }

    //Identify legal rightward moves
    //Queen can legally move right until encountering other piece
    //Consider all moves right till end of board
    for (int New_Column = column + 1; New_Column <= 7; New_Column++)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, row, New_Column, Board[row][New_Column].Piece_Colour, Board[row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, row, New_Column, Board[row][New_Column].Piece_Colour, Board[row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching right; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves right
            break;
        }
    }

    //Identify legal left up diagonal moves
    //Queen can legally move left up diagonally until encountering other piece
    //Consider all moves left up diagonal till end of board
    for (int New_Row = row - 1, New_Column = column - 1; New_Row >= 0 and New_Column >= 0; New_Row--, New_Column--)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching left up diagonal
            //; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves left up diagonal
            break;
        }
    }

    //Identify legal right up diagonal moves
    //Queen can legally move right up diagonally until encountering other piece
    //Consider all moves right up diagonal till end of board
    for (int New_Row = row - 1, New_Column = column + 1; New_Row >= 0 and New_Column <= 7; New_Row--, New_Column++)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching right up diagonal
            //; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves right up diagonal
            break;
        }
    }

    //Identify legal left down diagonal moves
    //Queen can legally move left down diagonally until encountering other piece
    //Consider all moves left down diagonal till end of board
    for (int New_Row = row + 1, New_Column = column - 1; New_Row <= 7 and New_Column >= 0; New_Row++, New_Column--)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching left down diagonal
            //; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves left down diagonal
            break;
        }
    }

    //Identify legal right down diagonal moves
    //Queen can legally move right down diagonally until encountering 
    //other piece
    //Consider all moves rigth down diagonal till end of board
    for (int New_Row = row + 1, New_Column = column + 1; New_Row <= 7 and New_Column <= 7; New_Row++, New_Column++)
    {
        //variable for defining a new legal move
        Move New_Move;
        //if square is unnocupied --> legal move
        if (Board[New_Row][New_Column].Is_Occupied == false)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
        }
        //if square is occupied by an opponent piece --> Legal Move
        else if (Board[New_Row][New_Column].Piece_Colour != Colour_Of_Piece)
        {
            //define this move
            Move New_Move = Define_Move(Colour_Of_Piece, 'Q', row, column, New_Row, New_Column, Board[New_Row][New_Column].Piece_Colour, Board[New_Row][New_Column].Piece_Type);
            //add it to Legal_Moves
            Legal_Moves.insert(Legal_Moves.end(), New_Move);
            //stop searching right down diagonal
            //; queen can not possibly move past this square
            break;
        }
        //if square is occupied by a teammate piece --> illegal move
        else
        {
            //do NOT include this move
            //stop searching for further moves right down diagonal
            break;
        }
    }

    //Return all legal moves for this queen piece
    return Legal_Moves;
}

//Legal Moves for knight piece
vector <Move> King_Legal_Moves(char Colour_Of_Piece, int row, int column)
{
    //Vector of moves for storing this pieces legal moves
    vector <Move> Legal_Moves;
    //king can move legally 1 square any direction onto
    //unnocupied or ocucpied by opponent squares
    //must consider that king can not step next to a king
    //identify all potential squares (1 square away) onto which the king could move
    for (int r = 0; r <= 7; r++)
    {
        for (int c = 0; c <= 7; c++)
        {
            //check if potential square is 1 square away from king
            if (((row - 1 <= r) and (r <= row + 1)) and ((column - 1 <= c) and (c <= column + 1))) // don't even ask; I think it works
            {
                //check if king can move there
                if ((Board[r][c].Is_Occupied == false) or (Board[r][c].Piece_Colour != Colour_Of_Piece))
                {
                    //define move since it is legal
                    Move New_Move = Define_Move(Colour_Of_Piece, 'K', row, column, r, c, Board[r][c].Piece_Colour, Board[r][c].Piece_Type);
                    //add it as a legal move
                    Legal_Moves.insert(Legal_Moves.end(), New_Move);
                }
            }
        }
    }
    /*
    //remove all squares which are adjacent to enemy king
    //loop through vector of legal_moves
    for (int i = 0; i < Legal_Moves.size(); i++)
    {
      for (int r = 0; r <= 7; r++)
      {
        for (int c = 0; c <= 7; c++)
        {
          //specify it be a square adjacent to a potential move_to square
          if (((Legal_Moves[i].End_Row - 1 <= r) and (r <= Legal_Moves[i].End_Row + 1)) and ((Legal_Moves[i].End_Column - 1 <= c) and (c <= Legal_Moves[i].End_Column + 1)) and ((r != Legal_Moves[i].End_Row) and (c != Legal_Moves[i].End_Column))) // don't even ask; I think it works
          {
            //check if this square has the enemy king
            if ((Board [r][c].Piece_Colour != Colour_Of_Piece) and (Board [r][c].Piece_Type = 'K'))
            {
              //since square has enemy king, this move is illegal
              //remove this move from vector
              Legal_Moves.erase (Legal_Moves.begin() + i);
            }
          }
        }
      }
    }
    */
    //Return all legal king moves
    return Legal_Moves;
}
//------------------------------------------------------------------------------------------------

//***Universal Chess Interface Protocol***
//------------------------------------------------------------------------------------------------
//Update internal board state as provided by Lichess
void Update_Board_State(string State_Provided_By_Lichess)
{
    //We need to parse the string, the board will be reset up completely every new move

    //Recognize that if the message is simply "position startpos", MateBot is playing as
    //white
    if (State_Provided_By_Lichess == "position startpos") MateBot_Playing_As = 'W';

    //Parse the string, splitting it up word by word; store each word into a vector of strings
    vector <string> State_Split_By_Words;
    string Individual_Command = "";
    for (int i = 0; i < State_Provided_By_Lichess.length(); i++)
    {
        //if this character is a space or end of command, store word collected so far (since last space) into vector
        if (State_Provided_By_Lichess[i] == ' ')
        {
            State_Split_By_Words.push_back(Individual_Command);
            //Reset word storing variable to empty
            Individual_Command = "";
        }
        //if useful character, save it into word variable
        else Individual_Command += State_Provided_By_Lichess[i];
    }
    //Also, add the last individual command to the array
    State_Split_By_Words.push_back(Individual_Command);

    //Set up board position by iterating through each command in vector
    for (int i = 0; i < State_Split_By_Words.size(); i++)
    {
        //if startpos command
        if (State_Split_By_Words[i] == "position") continue;
        //setup starting position
        else if (State_Split_By_Words[i] == "startpos") Board_Setup();
        else if (State_Split_By_Words[i] == "moves") continue;
        //conduct/make moves given
        //consider special moves of castling
        //if move is white short castilng
        else if (State_Split_By_Words[i] == "e1g1")
        {
            //Make move with king
            Make_Move(Convert_Move_From_UCI_Form("e1g1"));
            //make the rook move
            Make_Move(Convert_Move_From_UCI_Form("h1f1"));
        }
        //if move is white long castling
        else if (State_Split_By_Words[i] == "e1c1")
        {
            //Make move with king
            Make_Move(Convert_Move_From_UCI_Form("e1c1"));
            //make the rook move
            Make_Move(Convert_Move_From_UCI_Form("a1d1"));
        }
        //if move is black short castle
        else if (State_Split_By_Words[i] == "e8g8")
        {
            //Make move with king
            Make_Move(Convert_Move_From_UCI_Form("e8g8"));
            //make the rook move
            Make_Move(Convert_Move_From_UCI_Form("h8f8"));
        }
        //if move is black long castle
        else if (State_Split_By_Words[i] == "e8c8")
        {
            //Make move with king
            Make_Move(Convert_Move_From_UCI_Form("e8c8"));
            //make the rook move
            Make_Move(Convert_Move_From_UCI_Form("a8d8"));
        }
        //if just a regular move
        else Make_Move(Convert_Move_From_UCI_Form(State_Split_By_Words[i]));
    }
}

//Convert MateBot's top move to UCI form
string Convert_Move_To_UCI_Form(Move Move)
{
    //Variable for storing converted string
    string Converted_Move = "";

    //Form is long algebraic notation: king's pawn opening is "e2e4"
    //Because of how we represented the board, we need the mirror the rows
    //vertically flipped

    //Append starting column
    Converted_Move += (char(Move.Start_Column + 97));
    //Append starting row
    Converted_Move += char((7 - Move.Start_Row) + 49);
    //Append ending column
    Converted_Move += (char(Move.End_Column + 97));
    //Append ending row
    Converted_Move += char((7 - Move.End_Row) + 49);

    //in case of promotion, we must append a 'q' to the end of the move
    //promotion can be detected by a pawn moving to row 0 or 7
    if ((Move.Piece == 'P') and ((Move.End_Row == 0) or (Move.End_Row == 7)))
    {
        Converted_Move += 'q';
    }

    //Return converted move
    return Converted_Move;
}

//Convert UCI form moves to internal storage form
Move Convert_Move_From_UCI_Form(string Move_In_UCI_Form)
{
    //Variable for storing converted move
    Move Converted_Move;

    //For making a regular move with one piece
    //Convert starting column
    Converted_Move.Start_Column = (int)Move_In_UCI_Form[0] - 97;
    //Convert starting row
    Converted_Move.Start_Row = 7 - ((int)Move_In_UCI_Form[1] - 49);
    //convert ending column
    Converted_Move.End_Column = (int)Move_In_UCI_Form[2] - 97;
    //convert ending row
    Converted_Move.End_Row = 7 - ((int)Move_In_UCI_Form[3] - 49);
    //Define the key move information
    Converted_Move.Colour = Board[Converted_Move.Start_Row][Converted_Move.Start_Column].Piece_Colour;
    Converted_Move.Piece = Board[Converted_Move.Start_Row][Converted_Move.Start_Column].Piece_Type;
    Converted_Move.Colour_Captured = Board[Converted_Move.End_Row][Converted_Move.End_Column].Piece_Colour;
    Converted_Move.Piece_Captured = Board[Converted_Move.End_Row][Converted_Move.End_Column].Piece_Type;

    return Converted_Move;
}
//------------------------------------------------------------------------------------------------