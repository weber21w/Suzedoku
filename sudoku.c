 // Sudoku89: Sudoku game for the TI-89
 // Written by James Trimble <jimmit_t@yahoo.com>
 // 	Copyright 2005 James Trimble
 // UzeSudoku: port for Uzebox, 2024 Lee Weber
 //
 // This program is free software; you can redistribute it and/or modify
 // it under the terms of the GNU General Public License as published by
 // the Free Software Foundation; either version 2 of the License, or
 // (at your option) any later version.
 //
 // This program is distributed in the hope that it will be useful,
 // but WITHOUT ANY WARRANTY; without even the implied warranty of
 // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 // GNU General Public License for more details.
 //
 // You should have received a copy of the GNU General Public License
 // along with this program; if not, write to the Free Software
 // Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

 // If you decide to use some of this code in your own program please send me an email.
 // I'd love to know that something I wrote was helpful.

#define WITHLINES 0
#define T_OFFX 6
#define T_OFFY 1+(1-WITHLINES)
#define R_OFFX 12
#define R_OFFY 6
#define B_OFFX 6
#define B_OFFY 11-(1-WITHLINES)
#define L_OFFX 0
#define L_OFFY 6
#define SIDE 10
#define R_BOARDX 86
#define R_BOARDY 0
#define L_BOARDX 0
#define L_BOARDY 0
#define SIZE 9
#define SCORECOUNT 5

#define STATE_GAMEOVER	0
#define STATE_PAUSED	1
#define STATE_PLAYING	2

uint8_t gameboard[SIZE][SIZE];
uint8_t locked[SIZE][SIZE];
uint8_t History[SIZE][SIZE][SIZE];

uint8_t frames = 0;
uint8_t seconds=0;
uint8_t minutes=0;	
uint8_t gamestate = STATE_PAUSED;

void PostVsync(){
	if(++frames == 60){
		frames = 0;
		if(++seconds == 60){
			seconds = 0;
			if(++minutes == 99){
				minutes = 99;
		}
	}
	old_joypad1_status_lo = joypad1_status_lo;
	joypad1_status_lo = ReadButtons();
}

void DrawBlock(int x, int y){//Draws a tetravex piece without numbers
//	DrawLine(x,y,SIDE+x,y,A_NORMAL);
//	DrawLine(x,y,x,SIDE+y,A_NORMAL);
//	DrawLine(SIDE+x,SIDE+y,SIDE+x,y,A_NORMAL);
//	DrawLine(SIDE+x,SIDE+y,x,SIDE+y,A_NORMAL);

}

void DrawValues(int x, int y, int myPiece){//Draws the numbers in the correct place for a piece at x,y. Note: uses the SIDE, ?_OFFX, and ?_OFFY defines
	PrintChar(x, y, myPiece);
}

void DrawPiece(int row, int col, int myPiece){//Draws the piece with the values in place
		ErasePiece(row,col);
		DrawBlock(SIDE*col+L_BOARDX,SIDE*row+L_BOARDY);
		if(myPiece != -1)
			DrawValues(SIDE*col+L_BOARDX,SIDE*row+L_BOARDY,myPiece);
}

void DrawBoard(){//Draws the current gameboard to the screen
	ClearVram();

//	for(i=0; i<SIZE; i++){// Draw the playing board
//		for(j=0; j<SIZE; j++){
//			DrawBlock(SIDE*i,SIDE*j);
//		}
//	}

	for(i=0;i<SIZE;i++){// Draw the pieces
		for(j=0;j<SIZE;j++){
			DrawPiece(i,j,gameboard[i][j]);
		}
	}
}

uint8_t pickPiece(int row, int col){//Choose a piece from the possible legal pieces at this position
	uint8_t sum = 0;
	for(uint8_t i = 0; i < 9; i++)
		sum += History[row][col][i];

	if(sum == 0)
		return -1;

	uint8_t r = GetPrngNumber(0)%9;
	while (History[row][col][r] != 1){
		if(++r == 9)
			r = 0;
	}
	
	if(History[row][col][i] == 1)
		return i+1;
	
	return -1;
}

void FindChoices(int row, int col, int *choices){//Create a bit mask showing which pieces are allowed at this position
	int i,j,sqr,sqc,sum=0;

	for(i=0;i<SIZE;i++)
		choices[i] = 1;

	for(i=0;i<SIZE;i++){
		if(gameboard[i][col] != -1)
			choices[gameboard[i][col]-1] = 0;

		if(gameboard[row][i] != -1)
			choices[gameboard[row][i]-1] = 0;
	}

	sqr = row/3;
	sqc = col/3;
	
	for(i=3*sqr;i<(3*(sqr+1));i++){
		for(j=3*sqc;j<(3*(sqc+1));j++){
			if(gameboard[i][j] != -1)
				choices[gameboard[i][j]-1] = 0;
		}
	}
}

uint16_t GenerateSudoku(int position){//A recursive function which utilizes backtracking to generate the sudoku board
	// Generate the choices
	int i,row,col,currentOption;
	row = position / SIZE;
	col = position % SIZE;
	if(History[row][col][0] == -1){ // If this is the first time at this square
		// Create a list of available choices for this spot
		FindChoices(row,col, History[row][col]); //available);
	}
	currentOption = pickPiece(row,col);

	if(row == SIZE-1 && col == SIZE-1){//Return 1 if we've reached the end
		gameboard[row][col] = currentOption;
		return 1;
	}

	if(currentOption == -1){ // If there are no possible choices left we need to backtrack
		// Reset the choices for the current spot
		History[row][col][0] = -1;
		gameboard[row][col] = -1;
		return GenerateSudoku(position-1);
	}else{ // This choice seems to be working
		gameboard[row][col] = currentOption;
		History[row][col][currentOption-1]=0;
		return GenerateSudoku(position+1);
	}

}

void Initialize(){//resets the gameboard and the pieces
	int i,j,k,count=0;
	int row1, col1, row2, col2, pickMask;
	for(i=0;i<SIZE;i++){
		GetPrngNumber(0);
		for(j=0;j<SIZE;j++){
			gameboard[i][j] = -1;
			for(k=0;k<SIZE;k++)
				History[i][j][k] = -1;
		}
	}
	
	pickMask = GetPrngNumber(0)%4;
	
	for(i=0;i<SIZE && pickMask != 0; i++){
		for(j=0;j<SIZE;j++)
			locked[i][j] = 0;
	}
	
	for(uint16_t i=0;i<SIZE*SIZE;i++)
		locked[i] = pgm_read_byte(&locked_masks[(pickMask*SIZE*SIZE)+(i/8)])&(128>>(i%8));

	GenerateSudoku(0);

	for(uint16_t i=0;i<SIZE*SIZE;i++){
		if(!locked[i])
			gameboard[i] = -1;
	}

	DrawBoard();
}

void ToggleCursor(int row, int col){
//			FillLines2(&(WIN_RECT){SIDE*col+L_BOARDX+1,SIDE*row+L_BOARDY+SIDE-1, SIDE*col+L_BOARDX+SIDE-1,SIDE*row+L_BOARDY+SIDE-1},
//			&(WIN_RECT){SIDE*col+L_BOARDX+1,SIDE*row+L_BOARDY+1, SIDE*col+L_BOARDX+SIDE-1,SIDE*row+L_BOARDY+1},
//			&(SCR_RECT){{0, 0, 160, 93}},A_XOR);
}

void ErasePiece(int x, int y){
//	if(((x >= 3 && x < 6) && (y < 3 || y >= 6)) || ((x < 3 || x >= 6) && (y >= 3 && y < 6))){
//			ScrRectFill(&(SCR_RECT){{SIDE*y+L_BOARDX+1,SIDE*x+L_BOARDY+1,SIDE*y+L_BOARDX+SIDE-1,SIDE*x+L_BOARDY+SIDE-1}},&(SCR_RECT){{0,0,160,93}},A_NORMAL);
//			ScrRectFill(&(SCR_RECT){{SIDE*y+L_BOARDX+1,SIDE*x+L_BOARDY+1,SIDE*y+L_BOARDX+SIDE-1,SIDE*x+L_BOARDY+SIDE-1}},&(SCR_RECT){{0,0,160,93}},A_REVERSE);
//	}else{
//			ScrRectFill(&(SCR_RECT){{SIDE*y+L_BOARDX+1,SIDE*x+L_BOARDY+1,SIDE*y+L_BOARDX+SIDE-1,SIDE*x+L_BOARDY+SIDE-1}},&(SCR_RECT){{0,0,160,93}},A_REVERSE);
//	}
}

uint8_t ValidMove(int row, int col, int myPiece){
	for(uint8_t i=0; i < SIZE; i++){
		if(gameboard[i][col] == myPiece && i != row)
			return 0;
	}

	for(uint8_t j=0; j < SIZE; j++){
		if(gameboard[row][j] == myPiece && j != col)
			return 0;
	}
	
	uint8_t sqr = row/3;
	uint8_t sqc = col/3;
	
	for(uint8_t i=3*sqr;i<(3*(sqr+1));i++){
		for(uint8_t j=3*sqc;j<(3*(sqc+1));j++){
			if(gameboard[i][j] == myPiece && i != row && j != col)
				return 0;
		}
	}
	return 1;
}

uint8_t CheckWin(){
	for(uint16_t i=0;i<SIZE*SIZE;i++){
		if(gameboard[i] == -1)
			return 0;
	}
	return 1;
}

void Intro(){
	ClearVram();
	FadeIn(3,0);
	Print(10,10,"UzeSudoku 1.0"
	Print(10,13,"Lee Weber(D3thAdd3r) 2024")
	Print(10,16,"Original TI-89 version")
	Print(10,19,"James Trimble(jimmit_t@yahoo.com) 2005");
	Print(10,21,"Pour la fille qui me fait le sourire");
	WaitVsync(90);
	FadeOut(3,1);
}

void AddHighScore(){

}

void PrintScores(){
	ClearVram();
	Print(10,10,PSTR("High Scores:"));
	for(uint8_t j=0;j<SCORECOUNT;j++){
		//printf("%d. %-10s - %d:%02d\n", j+1, highScores[j].name, highScores[j].min, highScores[j].sec);
	}
}

//DO PCM MUSIC!!!!!!!!!!
int main(){
	int i,j;
	int row=0,col=0;
	int currentPiece;

	int numscores = 0;
	gamestate = STATE_PAUSED;

	GetPrngNumber(GetTrueRandomSeed());
	Intro();
	Title();
NEW_GAME:
	Initialize();
	frames = seconds = minutes = 0;// Reset timer
	gamestate = playing; // Start the clock
GAME_TOP:
	WaitVsync(1);
	if((joypad1_status_lo & BTN_START) && !(old_joypad1_status_lo & BTN_START)){
		if(gamestate == STATE_PAUSED){
			gamestate = playing;
			DrawBoard();
			ToggleCursor(row,col);
		}else{
			gamestate = STATE_PAUSED;
			ClearVram();
			Print(10,10"Game Paused");
			//TODO ADD PAUSE MENU OPTIONS HERE
		}
		continue;
	}
	if(gamestate == STATE_PAUSED)
		continue;


		if(key == BTN_UP){
			ToggleCursor(row,col);
			if(--row < 0)
				row = SIZE - 1;
				ToggleCursor(row,col);
			}else if(key == BTN_DOWN){
					ToggleCursor(row,col);
			if(++row > SIZE-1)
				row = 0;
			ToggleCursor(row,col);
		}else if(key == BTN_RIGHT){
			ToggleCursor(row,col);
			if(++col > SIZE-1)
				col = 0;
			ToggleCursor(row,col);
		}else if(key == BTN_LEFT){
			ToggleCursor(row,col);
			if(--col < 0)
				col = SIZE-1;
			ToggleCursor(row,col);
		}else if(key >= '0' && key <= '9'){
			currentPiece = key - '0';
			if(currentPiece == 0 && !(locked[row][col])){
				gameboard[row][col] = -1;
				ErasePiece(row, col);
				ToggleCursor(row,col);
			}else if(ValidMove(row,col,currentPiece) && !(locked[row][col])){
				gameboard[row][col] = currentPiece;
				DrawPiece(row,col,currentPiece);
				ToggleCursor(row,col);					
			}
	
			if(CheckWin()){
				ClearVram();
				gamestate = STATE_PAUSED; // Stop the clock
				Print(10,10,PSTR("YOU WIN!"));
				Print(10,11,PSTR("Enter name:"));
				//GetScores(highscores); // Populate the highscores list
				//UpdateScores(myScore, highscores); // Add the new entry to the list			
				gamestate = STATE_GAMEOVER; // End the game
			}
		}				
	goto GAME_TOP;
	return 0;
}
