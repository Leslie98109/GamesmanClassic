// $id$
// $log$

/* Above is will include the name and the log of the last
 * person to commit this file to gamesman.
 */

/************************************************************************
**
** NAME:        mwinkers.c
**
** DESCRIPTION: Winkers
**
** AUTHORS:     Newton Le, Edward Li - University of California Berkeley

**
** DATE:        02/26/2004
**
** UPDATE HIST: 
**
** 
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "hash.h"

extern STRING gValueString[];

POSITION gNumberOfPositions; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/

POSITION gInitialPosition; /* The initial position (starting board) */
POSITION gMinimalPosition; /* */
POSITION kBadPosition        = -1; /* A position that will never be used */

STRING   kGameName           = "Winkers"; /* The name of your game */
STRING   kDBName             = "Winkers"; /* The name to store the database under */
BOOLEAN  kPartizan           = TRUE; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kDebugMenu          = TRUE; /* TRUE while debugging */
BOOLEAN  kGameSpecificMenu   = FALSE; /* TRUE if there is a game specific menu*/
BOOLEAN  kTieIsPossible      = TRUE; /* TRUE if a tie is possible */
BOOLEAN  kLoopy               = FALSE; /* TRUE if the game tree will have cycles (a rearranger style game) */
BOOLEAN  kDebugDetermineValue = TRUE; /* TRUE while debugging */
void*	 gGameSpecificTclInit = NULL;

/* 
   Help strings that are pretty self-explanatory 
*/

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
""; 

STRING   kHelpOnYourTurn =
"";

STRING   kHelpStandardObjective =
"";

STRING   kHelpReverseObjective =
"";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"";

STRING   kHelpExample =
"";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

#define BOARDWIDTH     4
#define BOARDHEIGHT    1
#define PASSMOVE 0
int BOARDSIZE = BOARDHEIGHT * (2 * BOARDWIDTH + BOARDHEIGHT) + BOARDWIDTH;

typedef enum possibleBoardPieces {
  Blank, O, R, B
} BlankORB;

char *gBlankORBString[] = { "�", "O", "R", "B" };

char *gBoard, *LegendKey;
int *CP, *RW, *RN;

/*typedef struct moveValuesStruct {
 *  BlankORB piece;
 *  int moveIndex;
 *  int dummy;
 *} moveValues;
 *
 * typedef moveValues *moveValuesPtr;
 */

/*************************************************************************
**
** Below is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

/* Function prototypes here. */

int moveUnhash_index(MOVE);
int moveUnhash_dummy(MOVE);
BlankORB moveUnhash_piece(MOVE);
int ConvertToNumber(char*);
char MoveToCharacter(MOVE);
void PrintSpaces(int n);
int Legend (int n);

MOVE moveHash (int, int, BlankORB);

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

extern VALUE     *gDatabase;


/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
** 
************************************************************************/

void InitializeGame ()
{
  gBoard = (char *) SafeMalloc (BOARDSIZE * sizeof(char));
  LegendKey = (char *) SafeMalloc (BOARDSIZE * sizeof(char));
  RN = (int *) SafeMalloc (BOARDSIZE * sizeof(int));
  RW = (int *) SafeMalloc (BOARDSIZE * sizeof(int));
  CP = (int *) SafeMalloc (BOARDSIZE * sizeof(int));

  int x;
  for (x = 0; x < BOARDSIZE; x++) {
    LegendKey[x] = Legend(x+1);
  } 

  int half = (BOARDSIZE + 1) / 2;

  int piece_array[] = {'B', 0, half, 
		       'R' , 0, half, 
		       'O', 0, BOARDSIZE, 
		       '�', 0, BOARDSIZE, 
		       -1};

  gNumberOfPositions = generic_hash_init(BOARDSIZE, piece_array, 0);
  
  int i;
  for (i=0; i<BOARDSIZE; i++)
    gBoard[i] = '�';

  gInitialPosition = generic_hash(gBoard, 1);
  gMinimalPosition = gInitialPosition;

  //  printf("Number of Boards: %d", gNumberOfPositions);

  for (i = 0; i < BOARDSIZE; i++) {
    RN[i] = RowNumber(i);
    CP[i] = ColPosition(i);
    RW[i] = RowWidth(i);
  }
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

void DebugMenu ()
{
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
** 
************************************************************************/

void GameSpecificMenu ()
{
}

  
/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

void SetTclCGameSpecificOptions (options)
	int options[];
{
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
** 
** INPUTS:      POSITION thePosition : The old position
**              MOVE     theMove     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       Hash ()
**              Unhash ()
**	            LIST OTHER CALLS HERE
*************************************************************************/
POSITION DoMove (thePosition, theMove)
	POSITION thePosition;
	MOVE theMove;
{
  int player = whoseMove(thePosition);

  generic_unhash(thePosition, gBoard);

  if (theMove != PASSMOVE) {
    if (gBoard[theMove-1] == '�')
      gBoard[theMove-1] = 'O';
    else if (player == 1)
      gBoard[theMove-1] = 'R';
    else
      gBoard[theMove-1] = 'B';
  } 

  return (generic_hash(gBoard,(player==1)?2:1));
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
** 
** OUTPUTS:     POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
  BlankORB theBlankORB[BOARDSIZE];
  signed char c;
  int i;

  printf("\n\n\t----- Get Initial Position -----\n");
  printf("\n\tPlease input the position to begin with.\n");
  printf("\tNote that it should be in the following format:\n\n");
  printf("  - -\n O - -\n- R - -         <----- EXAMPLE \n - R O\n  - -\n\n");

  i = 0;
  getchar();
  while(i < BOARDSIZE && (c = getchar()) != EOF) {
    if(c == 'r' || c == 'R')
      gBoard[i++] = 'R';
    else if(c == 'o' || c == 'O' || c == '0')
      gBoard[i++] = 'O';
    else if(c == 'b' || c == 'B')
      gBoard[i++] = 'B';
    else if(c == '-')
      gBoard[i++] = '�';
    else
      ;   /* do nothing */
  }
  return(generic_hash(gBoard, 1));
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
** 
** INPUTS:      MOVE    computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove(computersMove, computersName)
	MOVE computersMove;
	STRING computersName;
{
  printf("%8s's move : ", computersName);
  if (computersMove == PASSMOVE)
    printf("pass");
  else 
    printf("%c", LegendKey[computersMove-1]);
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with Gobblet. Three in a row for the player
**              whose turn it is a win, otherwise its a loss.
**              Otherwise undecided.
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       LIST FUNCTION CALLS
**              
**
************************************************************************/

VALUE Primitive (pos)
	POSITION pos;
{
  BOOLEAN AllFilledIn();
  BlankORB ThreeInARow(), theBlankORB[BOARDSIZE];
  VALUE EndGame(char, int);
  generic_unhash(pos, gBoard);
  char current;

  int i;
  for (i = 0; i < BOARDSIZE; i++) {
    if (CP[i] < RW[i] - 2) {
      current = ThreeInARow(gBoard, i, i+1, i+2);
      if (current != '�') 
	return EndGame(current, whoseMove(pos));
    }

    if (RN[i] < BOARDHEIGHT - 1) {
      current = ThreeInARow(gBoard, i, i + RW[i], i + 2*RW[i] + 1);
      if (current != '�')
	return EndGame(current, whoseMove(pos));
    } else if (RN[i] == BOARDHEIGHT - 1 && CP[i] != 0) {
      current = ThreeInARow(gBoard, i, i + RW[i], i + 2*RW[i]);
      if (current != '�')
	return EndGame(current, whoseMove(pos));
    } else if (RN[i] >= BOARDHEIGHT && RN[i] <= 2*BOARDHEIGHT - 2 && CP[i] >=2) {
      current = ThreeInARow(gBoard, i, i + RW[i]-1, i + 2*RW[i] - 3);
      if (current != '�')
	return EndGame(current, whoseMove(pos));
    }

    if (RN[i] < BOARDHEIGHT - 1) {
      current = ThreeInARow(gBoard, i, i + RW[i] + 1, i + 2*RW[i] + 3);
      if (current != '�')
	return EndGame(current, whoseMove(pos));
    } else if (RN[i] == BOARDHEIGHT - 1 && CP[i] != RW[i]-1) {
      current = ThreeInARow(gBoard, i, i + RW[i]+1, i + 2*RW[i] + 2);
      if (current != '�')
	return EndGame(current, whoseMove(pos));
    } else if (RN[i] >= BOARDHEIGHT && RN[i] <= 2*BOARDHEIGHT - 2 && CP[i] < RW[i] -2) {
      current = ThreeInARow(gBoard, i, i + RW[i], i + 2*RW[i] - 1);
      if (current != '�')
	return EndGame(current, whoseMove(pos));
    }
  }
    if (AllFilledIn(gBoard))
      return tie;
    else 
      return undecided;
}

/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
**              prediction of the game's outcome.
** 
** INPUTS:      POSITION position   : The position to pretty print.
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()
**              LIST OTHER CALLS HERE
**
************************************************************************/

void PrintPosition (position, playerName, usersTurn)
	POSITION position;
	STRING playerName;
	BOOLEAN usersTurn;
{
  int i, j, m = 0, n = 0;
  generic_unhash(position, gBoard);

  printf("\n");

  for (i = 0; i < 2*BOARDHEIGHT+1; i++) {
    if (i == BOARDHEIGHT)
      printf (" LEGEND: ");
    else
      printf ("         ");

    PrintSpaces (abs(BOARDHEIGHT - i));

    for (j = 0; j < BOARDWIDTH + BOARDHEIGHT - abs(BOARDHEIGHT - i); j++)
      printf("%c ", LegendKey[n++]);
    
    PrintSpaces (abs(BOARDHEIGHT - i));
    printf(": ");
    PrintSpaces (abs(BOARDHEIGHT - i));
    
    for (j = 0; j < BOARDWIDTH + BOARDHEIGHT - abs(BOARDHEIGHT - i); j++)
      printf("%c ", gBoard[m++]);

    printf("\n");
  }
  
  printf("\n");

}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
**              from this position. Return a pointer to the head of the
**              linked list.
** 
** INPUTS:      POSITION position : The position to branch off of.
**
** OUTPUTS:     (MOVELIST *), a pointer that points to the first item  
**              in the linked list of moves that can be generated.
**
** CALLS:       GENERIC_PTR SafeMalloc(int)
**              LIST OTHER CALLS HERE
**
************************************************************************/
MOVELIST *GenerateMoves (position)
         POSITION position;
{
  MOVELIST *CreateMovelistNode(), *head = NULL;
  BOOLEAN AllFilledIn(char*);
  VALUE Primitive();
  int player = whoseMove (position);
  char wink, opWink;
  int numCheckers = 0;
  int numWinks = 0;
  int numOpWinks = 0;
  int i;

  //  if (Primitive(position)) {
    generic_unhash(position, gBoard);

    if (player == 1) {
      wink = 'R';
      opWink = 'B';
    } else {
      wink = 'B';
      opWink = 'R';
    }
    
    //Count pieces on board
    for (i = 0; i < BOARDSIZE; i++)
      if (gBoard[i] == opWink)
	numOpWinks++;
      else if (gBoard[i] == wink)
	numWinks++;
      else if (gBoard[i] == 'O')
	numCheckers++;
    
    /*    //Generate checker moves 
    if (((BOARDSIZE+1)/2 - numOpWinks - numCheckers/2) > 0)
      for (i = 0; i < BOARDSIZE; i--)
	if (gBoard[i] == '�')
	  head = CreateMovelistNode(moveHash(0, i , O), head);
    
    //Generate winker moves
    if (numWinks > 0)
      for (i = 0; i < BOARDSIZE; i--)
	if (gBoard[i] == O)
	  head = CreateMovelistNode(moveHash(0, i , wink), head);
    */

    for (i = BOARDSIZE - 1; i >= 0; i--) {
      if (gBoard[i] == 'O' && numWinks < (BOARDSIZE+1)/2)
	head = CreateMovelistNode(i+1, head);
      else if (gBoard[i] == '�' && ((BOARDSIZE+1)/2 - numOpWinks - numCheckers/2) > 0)
	head = CreateMovelistNode(i+1, head);
    }

    if (head == NULL && AllFilledIn(gBoard) == FALSE)
      head = CreateMovelistNode(PASSMOVE, head);

    return head;
    // }
}
 
/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
** 
** INPUTS:      POSITION *thePosition : The position the user is at. 
**              MOVE *theMove         : The move to fill with user's move. 
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (thePosition, theMove, playerName)
	POSITION thePosition;
	MOVE *theMove;
	STRING playerName;
{
  USERINPUT ret, HandleDefaultTextInput();
  do {
    printf("%8s's move [(p)ass,", playerName);
    if (BOARDSIZE == 0)
      printf("0] : ");
    else if (BOARDSIZE < 10)
      printf("1-%c] : ", LegendKey[BOARDSIZE-1]);
    else if (BOARDSIZE == 10)
      printf("1-9/A] : ");
    else if (BOARDSIZE < 36)
      printf("1-9/A-%c] : ", LegendKey[BOARDSIZE-1]);
    else if (BOARDSIZE == 36)
      printf("1-9/A-Z/a] : ");
    else 
      printf("1-9/A-Z/a-%c] : ", LegendKey[BOARDSIZE-1]);

    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if (ret != Continue)
      return(ret);

  } while(TRUE);

  return (Continue);
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              For example, if the user is allowed to select one slot
**              from the numbers 1-9, and the user chooses 0, it's not
**              valid, but anything from 1-9 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine.
** 
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (input)
	STRING input;
{
  if (strlen(input) != 1)
    return FALSE;
  int a;
  if (a = ConvertToNumber(input) < 0)
    return FALSE;
  return TRUE;
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**              No checking if the input is valid is needed as it has
**              already been checked!
** 
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove (input)
	STRING input;
{
  return ConvertToNumber(input);
}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
** 
** INPUTS:      MOVE *theMove         : The move to print. 
**
************************************************************************/

void PrintMove (move)
	MOVE move;
{
  if (move == PASSMOVE)
    printf("pass");
  else
    printf("%c", LegendKey[move-1]);
}

/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of option combinations
**				there are with all the game variations you program.
**
** OUTPUTS:     int : the number of option combination there are.
**
************************************************************************/

int NumberOfOptions()
{    
  return 2;
} 

/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function to keep track of all the game variants.
**				Should return a different number for each set of
**				variants.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption()
{
  if(gStandardGame) return 1 ;
  return 2 ;
} 

/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash for the game variants.
**				Should take the input and set all the appropriate
**				variants.
**
** INPUT:     int : the number representation of the options.
**
************************************************************************/

void setOption(int option)
{
  if(option == 1)
    gStandardGame = TRUE ;
  else
    gStandardGame = FALSE ;
}


/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/


/************************************************************************
** This is where you can put any helper functions, including your
** hash and unhash functions if you are not using one of the existing
** ones.
************************************************************************/

int ConvertToNumber(input)
     STRING input;
{
  char a = input[0];
  int i;
  for (i=0; i< BOARDSIZE; i++)
    if (a == LegendKey[i])
      return i+1;
  
  if (a == 'p' || a == 'P')
    return 0;

  return -1;
}

char MoveToCharacter(move)
     MOVE move;
{
  if (move < 10)
    return (move + '0');
  else if (move < 36)
    return (move + 'a' - 10);
  else 
    return (move + 'A' - 36);
}

/* PrintSpaces(n) outputs n spaces
 */
void PrintSpaces (n)
     int n;
{
  int i;
  for (i = 0; i < n; i++)
    printf(" ");
}

/* Legend will calculate the correct symbol to display on board.
 *
 * n - the position in numberical form
 * 
 * returns 0-9,a-z,A-Z
 */
int Legend (n)
     int n;
{
  if (n > 35)
    n += ('a' - 36);
  else if (n > 9)
    n += ('A' - 10);
  else
    n += '0';
  return n;
}

VALUE EndGame(char x, int player) {
  if (x == 'R') {
    if (player == 1)
      return win;
    else
      return lose;
  } else if (x == 'B') {
    if (player == 1)
      return lose;
    else
      return win;
  } else
    return undecided;
}


char ThreeInARow(theBlankORB, a, b, c)
     char theBlankORB[];
     int a, b, c;
{
  if (theBlankORB[a] == theBlankORB[b] && 
      theBlankORB[b] == theBlankORB[c] &&
      (theBlankORB[c] != '�' || theBlankORB[c] != 'O'))
    return theBlankORB[a];
  else 
    return '�';
}

BOOLEAN AllFilledIn(theBlankORB)
     char theBlankORB[];
{
  int i;

  for (i = 0; i < BOARDSIZE; i++) {
    if (theBlankORB[i]=='�' || theBlankORB[i]=='O')
      return FALSE;
  }
  return TRUE;
}

/*** RowNumber() returns the row number of the ith piece.
 * 
 * Rows are numbered starting with 0.
 *
 */

int RowNumber(i)
     int i;
{
  int CurrentRow = 0, CurrentPosition = 0, CurrentWidth = BOARDWIDTH;
  
  while(TRUE) {
    if (i < CurrentWidth)
      return CurrentRow;
    i -= CurrentWidth;
    if (CurrentRow < BOARDHEIGHT)
      CurrentWidth += 1;
    else
      CurrentWidth -= 1;
    CurrentRow += 1;
  }
  return 0;
}

/*** RowWidth() determines the width of the row i is on.
 *
 */

int RowWidth(i)
     int i;
{
  int CurrentRow = 0, CurrentPosition = 0, CurrentWidth = BOARDWIDTH;
  
  while(TRUE) {
    if (i < CurrentWidth)
      return CurrentWidth;
    i -= CurrentWidth;
    if (CurrentRow < BOARDHEIGHT)
      CurrentWidth += 1;
    else
      CurrentWidth -= 1;
    CurrentRow += 1;
  }
  return 0;
}

/*** ColPosition() determines where in the Row a piece is.
 *
 * The first piece in the row is 0.
 * The last piece is (width - 1).
 *
 */

int ColPosition(i)
     int i;
{
  int CurrentRow = 0, CurrentPosition = 0, CurrentWidth = BOARDWIDTH;
  
  while(TRUE) {
    if (i < CurrentWidth)
      return i;
    i -= CurrentWidth;
    if (CurrentRow < BOARDHEIGHT)
      CurrentWidth += 1;
    else
      CurrentWidth -= 1;
    CurrentRow += 1;
  }
  return 0;
}
  
