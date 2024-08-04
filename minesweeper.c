#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BOARD_WIDTH 30
#define BOARD_HEIGHT 20
#define COVERED_CELL '-'
#define FLAGGED_CELL 'F'
#define BLANK_CELL ' '

enum cover_cell { covered, uncovered, flagged };

int random_number(int min_num, int max_num)
{
	return (rand() % ((max_num + 1) - min_num)) + min_num;
}

void initialize_cover(enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT])
{
	for (int i = 0; i < BOARD_WIDTH; i++)
	{
		for (int j = 0; j < BOARD_HEIGHT; j++)
		{
			cover[i][j] = covered;
		}
	}
}

void initialize_board(char board[BOARD_WIDTH][BOARD_HEIGHT], int first_swept_x, int first_swept_y)
{
	// TODO: Make algorithm to initialize board on first sweep
	// Also store in git and separate into multiple files
	for (int x = 0; x < BOARD_WIDTH; x++)
	{
		for (int y = 0; y < BOARD_HEIGHT; y++)
		{
			bool space_is_mine = random_number(0, 4) == 0;
			if (space_is_mine)
			{
				board[x][y] = 'X';
			} else {
				board[x][y] = 'O';
			}
		}
	}
}

void toggle_flag(enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT], int x, int y)
{
	if (cover[x][y] == covered)
	{
		cover[x][y] = flagged;
	}
	else if (cover[x][y] == flagged)
	{
		cover[x][y] = covered;
	}
}

void sweep_cell(char board[BOARD_WIDTH][BOARD_HEIGHT], int x, int y, bool board_initialized)
{
	if (!board_initialized) { initialize_board(board, x, y); }
}

void draw_board(
	char board[BOARD_WIDTH][BOARD_HEIGHT],
	enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT],
	int target_x,
	int target_y
){
	/* Create the board box window */
	WINDOW* board_box_win = newwin(BOARD_HEIGHT + 2, BOARD_WIDTH * 2 + 1, 4, 0);
	refresh();
	/* Draw the surrounding box */
	box(board_box_win, 0, 0);

	/* Create the board box window */
	WINDOW* board_win = derwin(board_box_win, BOARD_HEIGHT, BOARD_WIDTH * 2, 1, 1);
	wrefresh(board_box_win);

	for (int x = 0; x < BOARD_WIDTH; x++)
	{
		for (int y = 0; y < BOARD_HEIGHT; y++)
		{
			/* Set highlight rules */
			if (x == target_x | y == target_y) { wattron(board_win, A_STANDOUT); }

			/* Print the cover  */
			switch (cover[x][y]) {
				case covered:
					mvwprintw(board_win, y, x * 2, "%c", COVERED_CELL);
					break;
				case flagged:
					mvwprintw(board_win, y, x * 2, "%c", FLAGGED_CELL);
					break;
				case uncovered:
					mvwprintw(board_win, y, x * 2, "%c", board[x][y]);
					break;
			}

			/* Unset highlight rules unless it's a row so we will highlight the next space as well */
			if (y != target_y) { wattroff(board_win, A_STANDOUT); }

			/* Print a space so we have a gap but it can still be highlighted */
			if (x < BOARD_WIDTH - 1) { mvwprintw(board_win, y, x * 2 + 1, " "); }

			/* Unset highlight rules */
			wattroff(board_win, A_STANDOUT);
		}
	}

	wrefresh(board_win);

	delwin(board_win);
	delwin(board_box_win);
}

void run_game_loop()
{
	/* Create the board and cover, initialize the cover, and set the status to uninitialized */
	char board[BOARD_WIDTH][BOARD_HEIGHT];
	enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT];
	initialize_cover(cover);
	bool board_initialized = false;
	/* Initialize the highlighted column and row */
	int target_x = BOARD_WIDTH / 2;
	int target_y = BOARD_HEIGHT / 2;

	bool running = true;
	while (running) {
		/* Print the instructions */
		mvprintw(0, 0, "Use the hjkl keys to move the cursor");
		mvprintw(1, 0, "Press q to quit");
		refresh();

		/* Draw the board and cover */
		draw_board(board, cover, target_x, target_y);

		/* Move the cursor back to the start */
		mvcur(0, 0, 0, 0);
		refresh();

		/* Get the user input */
		char input = getch();
		switch (input) {
			case 'h':
				/* Move left */
				if (target_x > 0) { target_x--; }
				break;
			case 'l':
				/* Move right */
				if (target_x < BOARD_WIDTH - 1) { target_x++; }
				break;
			case 'j':
				/* Move down */
				if (target_y < BOARD_HEIGHT - 1) { target_y++; }
				break;
			case 'k':
				/* Move up */
				if (target_y > 0) { target_y--; }
				break;
			case ' ':
				/* Sweep cell */
				sweep_cell(board, target_y, target_x, board_initialized);
				break;
			case 'f':
				/* Toggle flag */
				toggle_flag(cover, target_x, target_y);
				break;
			case 'q':
				/* Quit the game */
				running = false;
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	/* Seed the RNG */
	srand(time(NULL));

	/* Initialize the screen */
	initscr();

	/* Run the game loop */
	run_game_loop();

	/* Deallocate memory and end ncurses */
	endwin();
	return 0;
}
