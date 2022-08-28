/*                                                                           */
/* (C) 2022, Luca Uburti							                         */
/*           luca.uburti@gmail.com                                           */
/*                                                                           */



#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <unistd.h>
#include <c64.h>
#include <time.h>
#include <string.h>

#include "CC65-CampoMinato64.h"
#include "IT_lang.h"


unsigned int s = 0;
int r = 0, c = 0, rows = 0, cols = 0, count = 0, gameOver = 0;
int randNum, difficulty, bombDispersalProbability, bombsPlanted;
int x, y, i, nestedRuns = 0, anotherPassNeeded = 0, passCounter;
char ch, playAgain;

enum GAMESTATUS {
	lost, won, ingame
} gameStatus;


const int MIN_DIFFICULTY = 1;
const int MAX_DIFFICULTY = 5;
const int MAX_ROWS = 20;
const int MIN_ROWS = 5;
const int MAX_COLS = 35;
const int MIN_COLS = 5;


typedef struct cell {
	int isVisible;
	int hasBomb;
	int bombsNearby;
} Cell;

Cell** field;

void main(void)
{

	cprintf("6502-C Compiler.\r\n");

	sleep(1);
	clrscr();

	gameStatus = ingame;
	while (gameStatus == ingame) {

		clrscr();

		splashScreen();

		gameSetup();

		createField();

		plantBombs();
		calcBombsNearby();


		gamePlay();

		gameEnded();
	}


}

void splashScreen() {
	bgcolor(COLOR_BLACK);
	bordercolor(COLOR_BLACK);
	textcolor(COLOR_WHITE);

	printf(INTRO01);
	printf(INTRO02);
	printf(INTRO03);
	printf(INTRO04);
	printf(INTRO05);
	printf(INTRO06);
	printf(INTRO07);
	printf(INTRO08);
	printf(INTRO09);
	printf(INTRO10);
	printf(INTRO11);
	printf(INTRO12);
	printf(INTRO13);
	printf(INTRO14);
	printf(INTRO15);
	printf(INTRO16);
	printf(INTRO17);
	printf(INTRO18);
	printf(INTRO19);
	printf(INTRO20);
	printf(INTRO21);
	printf(INTRO22);
	printf(INTRO23);
	printf(INTRO24);

	cgetc();
	clrscr();

}

void gameSetup(void) {


	printf(INSERT_FIELD_DIMENSIONS);
	printf(INSERT_ROW_NUMBERS);
	printf(" (%d - %d)\n", MIN_ROWS, MAX_ROWS);
	checkIntInput(&rows, MIN_ROWS, MAX_ROWS);
	printf("\n");
	printf(INSERT_COL_NUMBERS);
	printf(" (%d - %d)\n", MIN_COLS, MAX_COLS);
	checkIntInput(&cols, MIN_COLS, MAX_COLS);
	printf("\n");
	printf(CHOOSE_DIFFICULTY);
	checkIntInput(&difficulty, MIN_DIFFICULTY, MAX_DIFFICULTY);

}

void checkIntInput(int* input, int min, int max) {
	scanf("%d", input);
	while (*input < min || *input > max) {
		printf(INCORRECT_INT_INPUT);
		printf(VALID_INT_INPUTS);
		printf("%d - %d\n", min, max);
		getchar(); //clunky scanf workaround
		scanf("%d", input);
	}
}

void createField(void) {
	//printf("Creazione di un campo di dimensioni %d x %d\n\n", rows, cols);

	field = (Cell**)malloc(rows * sizeof(Cell*));
	for (r = 0; r < rows; r++)
	{
		field[r] = (Cell*)malloc(cols * sizeof(Cell));
	}

	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) { //init stage, set all cell fields to 0
			field[r][c].isVisible = 0;
			field[r][c].hasBomb = 0;
			field[r][c].bombsNearby = 0;

		}
	}

}

void plantBombs(void) {

	bombDispersalProbability = difficulty * 3; //empirically tested, seems ok
	bombsPlanted = 0;

	while (bombsPlanted < 2) { // no matter how easy the game or small the field, we must have at least a couple bombs somewhere!
		for (r = 0; r < rows; r++) {
			for (c = 0; c < cols; c++) {
				_randomize(); // create pseudorandom seed
				randNum = rand() % 100;
				//printf("rand: %d\n", randNum);
				if (randNum < bombDispersalProbability) {
					if (field[r][c].hasBomb == 0) { //don't put 2 bombs in the same cell, besides, only 1 bomb per cell is supported in this game ;)
						field[r][c].hasBomb = 1;
						bombsPlanted++;
					}
				}
			}
		}
	}
	//printf("bombs planted %d\n", bombsPlanted);
}

void calcBombsNearby() {
	int bombCounter;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			bombCounter = 0;
			if ((r - 1 >= 0) && (c - 1 >= 0)) {	//upper left
				if (field[r - 1][c - 1].hasBomb == 1) {
					bombCounter++;
				}
			}
			if (r - 1 >= 0) {	//up
				if (field[r - 1][c].hasBomb == 1) {
					bombCounter++;
				}
			}
			if ((r - 1 >= 0) && (c + 1 < cols)) {	//upper right
				if (field[r - 1][c + 1].hasBomb == 1) {
					bombCounter++;
				}
			}
			if (c - 1 >= 0) {	// left
				if (field[r][c - 1].hasBomb == 1) {
					bombCounter++;
				}
			}
			if (c + 1 < cols) {	//right
				if (field[r][c + 1].hasBomb == 1) {
					bombCounter++;
				}
			}
			if ((r + 1 < rows) && (c - 1 >= 0)) { //lower left
				if (field[r + 1][c - 1].hasBomb == 1) {
					bombCounter++;
				}
			}
			if (r + 1 < rows) { //down
				if (field[r + 1][c].hasBomb == 1) {
					bombCounter++;
				}
			}
			if ((r + 1 < rows) && (c + 1 < cols)) { //lower right
				if (field[r + 1][c + 1].hasBomb == 1) {
					bombCounter++;
				}
			}

			field[r][c].bombsNearby = bombCounter;
		}
	}
}

void gamePlay(void) {
	clrscr();
	displayField();
	gotoxy(0, 0);

	while (gameStatus == ingame) {
		x = wherex();
		y = wherey();
		cursor(1); //blink

		textcolor(COLOR_GRAY1);
		gotoxy(0, 22);
		printf("Muovi il cursore con i tasti W-A-S-D.\nScopri una cella con il tasto X");
		textcolor(COLOR_WHITE);
		gotoxy(0, 24);
		printf("x:%d y:%d                            ", x + 1, y + 1);
		gotoxy(x, y);


		switch (cgetc()) {
		case 'w':
			if (y > 0) {
				gotoxy(x, --y);
				redrawCell(y + 1, x);
				gotoxy(0, 24);
				printf("x:%d y:%d                            ", x + 1, y + 1);
				gotoxy(x, y);

			}
			break;
		case 'a':
			if (x > 0) {
				gotoxy(--x, y);
				redrawCell(y, x+1);
				gotoxy(0, 24);
				printf("x:%d y:%d                            ", x + 1, y + 1);
				gotoxy(x, y);
			}
			break;
		case 's':
			if (y < rows - 1) {
				gotoxy(x, ++y);
				redrawCell(y - 1, x);
				gotoxy(0, 24);
				printf("x:%d y:%d                            ", x + 1, y + 1);
				gotoxy(x, y);
			}
			break;
		case 'd':
			if (x < cols - 1) {
				gotoxy(++x, y);
				redrawCell(y, x-1);
				gotoxy(0, 24);
				printf("x:%d y:%d                            ", x + 1, y + 1);
				gotoxy(x, y);
			}
			break;
		case 'x':	//note: y=rows - x=cols

			if (field[y][x].isVisible == 1) {
				break; // cell already uncovered, do nothing
			}

			field[y][x].isVisible = 1;

			if (field[y][x].hasBomb == 1) { //we died, gameover	
				gameStatus = lost;

				bordercolor(COLOR_RED);

				clrscr();
				displayField();//with gameStatus lost it will now show the whole field

				gotoxy(0, 23);
				textcolor(COLOR_RED);
				printf("                  BOOM!                ");
				textcolor(COLOR_WHITE);
				gotoxy(0, 23);

				for (i = 0; i < 1; i++) {	//make the exploded bomb stand out
					gotoxy(x, y);
					textcolor(COLOR_RED);
					revers(0);
					printf(" ");
					sleep(1);
					gotoxy(x, y);
					revers(1);
					printf("*");
					sleep(1);
				}
				textcolor(COLOR_WHITE);


			}
			else if (field[y][x].bombsNearby > 0) { //non zero cell discovered
				clrscr();
				displayField();
				gotoxy(0, 24);
				printf("OK! x:%d y:%d  ", field[y][x].bombsNearby, x + 1, y + 1);
				gotoxy(x, y);
				checkWinCondition();
			}
			else { //zero cell
				//discoverEmptyCellsNearbyRecursive(y, x);  //recursive function not good: on big fields easily recurses 100 times and overflows the stack
				discoverEmptyCellsNearbyIterative(y, x);
				clrscr();
				displayField();
				gotoxy(0, 24);
				printf("OK! x:%d y:%d  ", x + 1, y + 1);
				gotoxy(x, y);
				checkWinCondition();
			}

			break;
		}

	}

}

void redrawCell(int r, int c) { //cursor "bug" workaround, the cursor leaves a black characters trail in its wake
	cursor(0);
	gotoxy(c, r);
	revers(1);
	if (field[r][c].isVisible == 0) {
		textcolor(COLOR_WHITE);
		printf("+");
	}
	else {
		switch (field[r][c].bombsNearby) {
		case 0:
			textcolor(COLOR_WHITE);
			break;
		case 1:
			textcolor(COLOR_CYAN);
			break;
		case 2:
			textcolor(COLOR_GREEN);
			break;
		case 3:
			textcolor(COLOR_BLUE);
			break;
		case 4:
			textcolor(COLOR_PURPLE);
			break;
		case 5:
			textcolor(COLOR_LIGHTRED);
			break;
		case 6:
			textcolor(COLOR_ORANGE);
			break;
		case 7:
			textcolor(COLOR_BROWN);
			break;
		case 8:
			textcolor(COLOR_BROWN);
			break;
		}
		if (field[r][c].bombsNearby > 0) {
			printf("%d", field[r][c].bombsNearby);
		}
		else {
			printf(" ");
		}

	}
	textcolor(COLOR_WHITE);
	cursor(1);
	revers(0);
}



void displayField() {
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			revers(1);
			if (gameStatus == ingame && field[r][c].isVisible == 0) {
				printf("+");
				continue; //cell not visible, skip all the other checks
			}

			if (field[r][c].hasBomb == 1) {
				textcolor(COLOR_RED);
				printf("B");
			}
			else {
				switch (field[r][c].bombsNearby) {
				case 0:
					textcolor(COLOR_WHITE);
					break;
				case 1:
					textcolor(COLOR_CYAN);
					break;
				case 2:
					textcolor(COLOR_GREEN);
					break;
				case 3:
					textcolor(COLOR_BLUE);
					break;
				case 4:
					textcolor(COLOR_PURPLE);
					break;
				case 5:
					textcolor(COLOR_LIGHTRED);
					break;
				case 6:
					textcolor(COLOR_ORANGE);
					break;
				case 7:
					textcolor(COLOR_BROWN);
					break;
				case 8:
					textcolor(COLOR_BROWN);
					break;
				}
				if (field[r][c].bombsNearby > 0) {
					printf("%d", field[r][c].bombsNearby);
				}
				else {
					printf(" ");
				}

			}
			textcolor(COLOR_WHITE);
		}
		printf("\n"); //end of row reached
	}
	revers(0);
}

void discoverEmptyCellsNearbyIterative(int r, int c) {
	//scan all the field uncovering cells that can be uncovered, needs multiple passes but no risky stack overflows
	anotherPassNeeded = 1;
	passCounter = 0;
	gotoxy(0, 24);
	printf(UNCOVERING_PLEASE_WAIT);
	while (anotherPassNeeded != 0) {
		printf(".");
		sleep(1);
		anotherPassNeeded = 0; //let's start by being optimistic
		for (r = 0; r < rows; r++) {
			for (c = 0; c < cols; c++) {
				if ((r - 1 >= 0) && (c - 1 >= 0)) {	//upper left
					if (field[r - 1][c - 1].isVisible == 1 && field[r - 1][c - 1].bombsNearby == 0 && field[r][c].isVisible == 0) {
						//printf("uncovering cells %d,%d\n", r, c);
						field[r][c].isVisible = 1;
						if (field[r][c].bombsNearby == 0) {
							anotherPassNeeded++;
						}
					}
				}
				if (r - 1 >= 0) {	//up
					if (field[r - 1][c].isVisible == 1 && field[r - 1][c].bombsNearby == 0 && field[r][c].isVisible == 0) {
						//printf("uncovering cells %d,%d\n", r, c);
						field[r][c].isVisible = 1;
						if (field[r][c].bombsNearby == 0) {
							anotherPassNeeded++;
						}
					}
				}
				if ((r - 1 >= 0) && (c + 1 < cols)) {	//upper right
					if (field[r - 1][c + 1].isVisible == 1 && field[r - 1][c + 1].bombsNearby == 0 && field[r][c].isVisible == 0) {
						//printf("uncovering cells %d,%d\n", r, c);
						field[r][c].isVisible = 1;
						if (field[r][c].bombsNearby == 0) {
							anotherPassNeeded++;
						}
					}
				}
				if (c - 1 >= 0) {	// left
					if (field[r][c - 1].isVisible == 1 && field[r][c - 1].bombsNearby == 0 && field[r][c].isVisible == 0) {
						//printf("uncovering cells %d,%d\n", r, c);
						field[r][c].isVisible = 1;
						if (field[r][c].bombsNearby == 0) {
							anotherPassNeeded++;
						}
					}
				}
				if (c + 1 < cols) {	//right
					if (field[r][c + 1].isVisible == 1 && field[r][c + 1].bombsNearby == 0 && field[r][c].isVisible == 0) {
						//printf("uncovering cells %d,%d\n", r, c);
						field[r][c].isVisible = 1;
						if (field[r][c].bombsNearby == 0) {
							anotherPassNeeded++;
						}
					}
				}
				if ((r + 1 < rows) && (c - 1 >= 0)) { //lower left
					if (field[r + 1][c - 1].isVisible == 1 && field[r + 1][c - 1].bombsNearby == 0 && field[r][c].isVisible == 0) {
						//printf("uncovering cells %d,%d\n", r, c);
						field[r][c].isVisible = 1;
						if (field[r][c].bombsNearby == 0) {
							anotherPassNeeded++;
						}
					}
				}
				if (r + 1 < rows) { //down
					if (field[r + 1][c].isVisible == 1 && field[r + 1][c].bombsNearby == 0 && field[r][c].isVisible == 0) {
						//printf("uncovering cells %d,%d\n", r, c);
						field[r][c].isVisible = 1;
						if (field[r][c].bombsNearby == 0) {
							anotherPassNeeded++;
						}
					}
				}
				if ((r + 1 < rows) && (c + 1 < cols)) { //lower right
					if (field[r + 1][c + 1].isVisible == 1 && field[r + 1][c + 1].bombsNearby == 0 && field[r][c].isVisible == 0) {
						//printf("uncovering cells %d,%d\n", r, c);
						field[r][c].isVisible = 1;
						if (field[r][c].bombsNearby == 0) {
							anotherPassNeeded++;
						}
					}
				}

			}
		}
	}

}

void discoverEmptyCellsNearbyRecursive(int r, int c) { //DO NOT use this! easily recurses over 100 times! risky stack overflows!!
	//if nearby cell can be uncovered it calls itself

	printf("uncovering cell %d %d \n", r, c);
	printf("nestedRuns; %d\n", nestedRuns);
	sleep(1);

	if ((r - 1 >= 0) && (c - 1 >= 0)) {	//upper left
		if (field[r - 1][c - 1].isVisible != 1) {
			field[r - 1][c - 1].isVisible = 1;
			if (field[r - 1][c - 1].bombsNearby == 0) {
				nestedRuns++;
				discoverEmptyCellsNearbyRecursive(r - 1, c - 1);
				nestedRuns--;
			}
		}
	}
	if (r - 1 >= 0) {	//up
		if (field[r - 1][c].isVisible != 1) {
			field[r - 1][c].isVisible = 1;
			if (field[r - 1][c].bombsNearby == 0) {
				nestedRuns++;
				discoverEmptyCellsNearbyRecursive(r - 1, c);
				nestedRuns--;
			}
		}
	}
	if (r - 1 >= 0 && c + 1 < cols) {	//upper right
		if (field[r - 1][c + 1].isVisible != 1) {
			field[r - 1][c + 1].isVisible = 1;
			if (field[r - 1][c + 1].bombsNearby == 0) {
				nestedRuns++;
				discoverEmptyCellsNearbyRecursive(r - 1, c + 1);
				nestedRuns--;
			}
		}
	}
	if (c - 1 >= 0) {	//left
		if (field[r][c - 1].isVisible != 1) {
			field[r][c - 1].isVisible = 1;
			if (field[r][c - 1].bombsNearby == 0) {
				nestedRuns++;
				discoverEmptyCellsNearbyRecursive(r, c - 1);
				nestedRuns--;
			}
		}
	}
	if (c + 1 < cols) {	//right
		if (field[r][c + 1].isVisible != 1) {
			field[r][c + 1].isVisible = 1;
			if (field[r][c + 1].bombsNearby == 0) {
				nestedRuns++;
				discoverEmptyCellsNearbyRecursive(r, c + 1);
				nestedRuns--;
			}
		}
	}
	if (r + 1 < rows && c - 1 >= 0) {	//lower left 
		if (field[r + 1][c - 1].isVisible != 1) {
			field[r + 1][c - 1].isVisible = 1;
			if (field[r + 1][c - 1].bombsNearby == 0) {
				nestedRuns++;
				discoverEmptyCellsNearbyRecursive(r + 1, c - 1);
				nestedRuns--;
			}
		}
	}
	if (r + 1 < rows) {	//down
		if (field[r + 1][c].isVisible != 1) {
			field[r + 1][c].isVisible = 1;
			if (field[r + 1][c].bombsNearby == 0) {
				nestedRuns++;
				discoverEmptyCellsNearbyRecursive(r + 1, c);
				nestedRuns--;
			}
		}
	}
	if (r + 1 < rows && c + 1 < cols) {	//lower right
		if (field[r + 1][c + 1].isVisible != 1) {
			field[r + 1][c + 1].isVisible = 1;
			if (field[r + 1][c + 1].bombsNearby == 0) {
				nestedRuns++;
				discoverEmptyCellsNearbyRecursive(r + 1, c + 1);
				nestedRuns--;
			}
		}
	}

}

void checkWinCondition() {
	gameStatus = won; //let's start optimistic
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			if (field[r][c].isVisible == 0 && field[r][c].hasBomb == 0) {
				gameStatus = ingame; //nope, still work to do
				break;
			}
		}
		if (gameStatus == ingame) {
			break;  //this row still had cells to be uncovered, don't bother checking other rows
		}
	}
}


void gameEnded() {

	if (gameStatus == won) {
		bordercolor(COLOR_GREEN);
		gotoxy(0, 23);
		textcolor(COLOR_GREEN);
		printf(WON_MSG);
		textcolor(COLOR_WHITE);
	}
	if (gameStatus == lost) {
		gotoxy(0, 23);
		printf(LOST_MSG);
	}
	sleep(3);
	gotoxy(0, 23);
	printf(PLAY_AGAIN);

	playAgain = cgetc();
	if ((playAgain == 'y') || (playAgain == 'Y')) {
		gameStatus = ingame;
	}
	else {
		clrscr();
		printf("Ciao!");
	}
	revers(0);
}
