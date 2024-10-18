//Eagle Eye Chess Engine
//Mate Csapo; July 2024
//Source File

//***PREAMBLE***
//------------------------------------------------------------------------------------------------
//Include iostream  
#include <iostream>
//Include file stream
#include <fstream>
//Include c++ sort() algorithm
#include <algorithm>
//Vector Library
#include <vector>
//Header File
#include "Eagle_Eye.hpp"

//Standard Namespace
using namespace std;
//------------------------------------------------------------------------------------------------

//***Board Storage***
//------------------------------------------------------------------------------------------------
class Chess_Board
{
    public:
        //Flag for which colour Eagle Eye is -- white = true; black = false
        bool Eagle_Eye_Colour = false;

        //Bitboards for storing piece locations
        //White pieces
        uint64_t White_Pawns;
        uint64_t White_Knights;
        uint64_t White_Bishops;
        uint64_t White_Rooks;
        uint64_t White_Queen;
        uint64_t White_King;

        //Black pieces
        uint64_t Black_Pawns;
        uint64_t Black_Knights;
        uint64_t Black_Bishops;
        uint64_t Black_Rooks;
        uint64_t Black_Queen;
        uint64_t Black_King;

        //Flags for if castling possible
        bool White_Can_Castle = true;
        bool Black_Can_Castle = false;

        //Updating board state from game manager
        void Update_State (string UCI_Game_State_Command)
        {
            //Set up starting position
            
        }
};

//Create a globally-accessible Chess_Board object for internally storing the game state
Chess_Board Board;
//------------------------------------------------------------------------------------------------

//***UCI Protocol***
//------------------------------------------------------------------------------------------------
void Execute_UCI_Command()
{
    //Input command from game manager
    string Command;
    getline (cin, Command);

    //Identify if Mate_Bot is playing as white or black
    if (Command == "position startpos") Board.Eagle_Eye_Colour = true;

    //Execute correct responses to UCI command
    if (Command == "uci") cout << "id name MateBot\nid author MVC\nuciok\n";
    else if (Command == "isready") cout << "readyok\n";
    else if (Command.substr(0, 8) == "position") Board.Update_State(Command);
}
//------------------------------------------------------------------------------------------------

//***Driver Function***
//------------------------------------------------------------------------------------------------
//Handles UCI interfacing and communication; calls for MateBot search to give top move
int main()
{
    //Receive and execute UCI commands from game manager
    while (1==1)
    {
        Execute_UCI_Command();
    }
}
//------------------------------------------------------------------------------------------------
