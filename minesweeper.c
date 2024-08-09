#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BOARD_WIDTH 30
#define BOARD_HEIGHT 20
#define INITIAL_MINE_COUNT 120
#define FIRST_SWEEP_SAFE_RADIUS 2

#define COVERED_CELL '-'
#define FLAGGED_CELL 'F'
#define BLANK_CELL ' '
#define MINE 'X'

#define INPUT_LEFT 'h'
#define INPUT_RIGHT 'l'
#define INPUT_UP 'k'
#define INPUT_DOWN 'j'
#define INPUT_FLAG 'f'
#define INPUT_SWEEP ' '
#define INPUT_QUIT 'q'

#define BOARD_OFFSET_TOP 6

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

bool cell_is_within_radius(int center_x, int center_y, int cell_x, int cell_y, int radius)
{
	/* Get the distance from the center in the x direction and return false if it's larger than the radius */
	int x_distance = center_x - cell_x;
	if (x_distance < 0) x_distance *= -1;
	if (x_distance > radius) return false;

	/* Get the distance from the center in the y direction and return false if it's larger than the radius */
	int y_distance = center_y - cell_y;
	if (y_distance < 0) y_distance *= -1;
	if (y_distance > radius) return false;

	/* If the cell is within the radius in both the x and y directions, return true */
	return true;
}

void place_mines(
	char board[BOARD_WIDTH][BOARD_HEIGHT],
	enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT],
	int first_swept_x,
	int first_swept_y
)
{
	int mines_to_place = INITIAL_MINE_COUNT;

	while (mines_to_place > 0)
	{
		/* Generate coordinates for the prospective mine */
		int mine_x = random_number(0, BOARD_WIDTH);
		int mine_y = random_number(0, BOARD_HEIGHT);

		/* Mines cannot be placed within a certain radius of the first cell swept */
		if (cell_is_within_radius(first_swept_x, first_swept_y, mine_x, mine_y, FIRST_SWEEP_SAFE_RADIUS)) continue;

		/* Don't place a mine where there is already a mine */
		if (board[mine_x][mine_y] == MINE) continue;

		/* Place a mine and decrement the number of mines remaining */
		/*
		 * BUG: The following line for some reason sometimes sets the cover in its space to blank or X.
		 * The board value can be anything. I have no idea why this happens.
		 */
		board[mine_x][mine_y] = MINE;
		mines_to_place--;
	}

	/* This is to "fix" the bug described above. It is not very elegant but it mostly works. */
	/* If you know of a better way to fix this, please let me know. It bothers me. */
	for (int x = 0; x < BOARD_WIDTH; x++)
	{
		for (int y = 0; y < BOARD_HEIGHT; y++)
		{
			switch (cover[x][y]) {
				case covered:
					break;
				case uncovered:
					break;
				case flagged:
					break;
				default:
					cover[x][y] = covered;
			}
		}
	}
}

int get_adjacent_mine_count(char board[BOARD_WIDTH][BOARD_HEIGHT], int cell_x, int cell_y)
{
	int adjacent_mine_count = 0;

	for (int adjacent_cell_x = cell_x - 1; adjacent_cell_x <= cell_x + 1; adjacent_cell_x++)
	{
		for (int adjacent_cell_y = cell_y - 1; adjacent_cell_y <= cell_y + 1; adjacent_cell_y++)
		{
			/* Skip this pass if we are out of bounds */
			if (
				(adjacent_cell_x < 0) |
				(adjacent_cell_y < 0) |
				adjacent_cell_x >= BOARD_WIDTH |
				adjacent_cell_y >= BOARD_HEIGHT
			)
			{
				continue;
			}

			/* Increment the count if this cell contains a mine */
			if (board[adjacent_cell_x][adjacent_cell_y] == MINE) adjacent_mine_count++;
		}
	}

	return adjacent_mine_count;
}

void initialize_board(
	char board[BOARD_WIDTH][BOARD_HEIGHT],
	enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT],
	int first_swept_x,
	int first_swept_y
)
{
	/* First set every cell to blank */
	for (int x = 0; x < BOARD_WIDTH; x++)
	{
		for (int y = 0; y < BOARD_HEIGHT; y++)
		{
			board[x][y] = BLANK_CELL;
		}
	}
	
	/* Place the mines */
	place_mines(board, cover, first_swept_x, first_swept_y);

	/* Assign numbers */
	char single_digit_int_chars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	for (int x = 0; x < BOARD_WIDTH; x++)
	{
		for (int y = 0; y < BOARD_HEIGHT; y++)
		{
			if (board[x][y] == MINE) { continue; }
			char adjacent_mines = single_digit_int_chars[get_adjacent_mine_count(board, x, y)];
      			if (adjacent_mines == '0') continue;
			board[x][y] = adjacent_mines;
		}
	}
}

void toggle_flag(enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT], int x, int y, int *p_mine_count)
{
	if (cover[x][y] == covered && p_mine_count > 0)
	{
		cover[x][y] = flagged;
		*p_mine_count = *p_mine_count - 1;
	}
	else if (cover[x][y] == flagged)
	{
		cover[x][y] = covered;
		*p_mine_count = *p_mine_count + 1;
	}
}

void uncover_cell(
	char board[BOARD_WIDTH][BOARD_HEIGHT],
	enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT],
	int x,
	int y,
	bool *p_game_over
)
{
	/* Only uncover if this is a covered cell */
	if (cover[x][y] != covered) return;

	/* Set this cell to uncovered */
	cover[x][y] = uncovered;

	/* If this is a mine, end the game */
	if (board[x][y] == MINE) *p_game_over = true;

	/* If this is not a blank cell then return here */
	if (board[x][y] != BLANK_CELL) { return; }

	for (int chain_x = x - 1; chain_x <= x + 1; chain_x++)
	{
		for (int chain_y = y - 1; chain_y <= y + 1; chain_y++)
		{
			/* Don't try to uncover out-of-bounds cells */
			if (chain_x < 0 | chain_y < 0 | chain_x >= BOARD_WIDTH | chain_y >= BOARD_HEIGHT) { continue; }
			uncover_cell(board, cover, chain_x, chain_y, p_game_over);
		}
	}
}

void sweep_cell(
	char board[BOARD_WIDTH][BOARD_HEIGHT],
	enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT],
	int x,
	int y,
	bool *p_board_initialized,
	bool *p_game_over
)
{
	/* Initialize the board if we haven't yet */
	if (!*p_board_initialized)
	{
		initialize_board(board, cover, x, y);
		*p_board_initialized = true;
	}

	/* Don't do anything if this cell is not covered */
	if (cover[x][y] != covered) return;

	uncover_cell(board, cover, x, y, p_game_over);
}

void draw_board(
	char board[BOARD_WIDTH][BOARD_HEIGHT],
	enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT],
	int target_x,
	int target_y,
	bool game_over
)
{
	/* Create the board box window */
	WINDOW* board_box_win = newwin(BOARD_HEIGHT + 2, BOARD_WIDTH * 2 + 1, BOARD_OFFSET_TOP, 0);
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
			if (x == target_x | y == target_y) wattron(board_win, A_STANDOUT);

			/* Move the cursor to the cell */
			wmove(board_win, y, x * 2);

			if (game_over) {
				wprintw(board_win, "%c", board[x][y]);
			}
			else
			{
				/* Print the cover  */
				switch (cover[x][y]) {
					case covered:
						wprintw(board_win, "%c", COVERED_CELL);
						break;
					case flagged:
						wprintw(board_win, "%c", FLAGGED_CELL);
						break;
					case uncovered:
						wprintw(board_win, "%c", board[x][y]);
						break;
					default:
						wprintw(board_win, "%c", '!');
				}
			}
			/* TODO: Maybe add a symbol for incorrect flag */


			/* Unset highlight rules unless it's a row so we will highlight the next space as well */
			if (y != target_y) wattroff(board_win, A_STANDOUT);

			/* Print a space so we have a gap but it can still be highlighted */
			if (x < BOARD_WIDTH - 1) mvwprintw(board_win, y, x * 2 + 1, " ");

			/* Unset highlight rules */
			wattroff(board_win, A_STANDOUT);
		}
	}

	wrefresh(board_win);

	delwin(board_win);
	delwin(board_box_win);
}

bool check_for_win(char board[BOARD_WIDTH][BOARD_HEIGHT], enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT])
{
	for (int x = 0; x < BOARD_WIDTH; x++)
	{
		for (int y = 0; y < BOARD_HEIGHT; y++)
		{
			/* If there is a remaining covered cell that is not a mine, victory has not been achieved */
			if (cover[x][y] == covered && board[x][y] != MINE) return false;
		}
	}

	return true;
}

void run_game_loop()
{
	/* Create the board and cover, initialize the cover, and set the status to uninitialized */
	char board[BOARD_WIDTH][BOARD_HEIGHT];
	enum cover_cell cover[BOARD_WIDTH][BOARD_HEIGHT];
	initialize_cover(cover);
	/* Set inital values */
	int target_x = BOARD_WIDTH / 2;
	int target_y = BOARD_HEIGHT / 2;
	int mine_count = INITIAL_MINE_COUNT;
	bool board_initialized = false;
	bool game_over = false;
	bool game_won = false;

	bool running = true;
	while (running) {
		/* Check for a win */
		if (check_for_win(board, cover)) game_won = true;

		/* TODO: Add reset function and keybind */
		/* Print the instructions */
		mvprintw(0, 0, "Use the hjkl keys to move the cursor, SPACE to sweep, and f to place or remove a flag.");
		mvprintw(1, 0, "Press q to quit");
		mvprintw(3, 0, "Mine Count: %d", mine_count);
		/* Print the face */
		if (game_over)
		{
			mvprintw(4, 0, ":(");
		}
		else
		{
			mvprintw(4, 0, ":)");
		}
		/* Print the Game Over/You Win message */
		/* TODO: Test if the win condition works and also add game over message */
		if (game_won) mvprintw(5, 0, "You Win!");
		refresh();
		/* TODO: Add more controls messages, add timer */

		/* Draw the board and cover */
		draw_board(board, cover, target_x, target_y, game_over);

		/* Move the cursor back to the start */
		mvcur(0, 0, 0, 0);
		refresh();

		/* Get user input */
		char input = getch();

		/* Only allow certain inputs after game over */
		if ((game_over || game_won) && input != INPUT_QUIT) continue;

		/* Handle user input */
		switch (input) {
			case INPUT_QUIT:
				/* Quit the game */
				running = false;
				break;
			case INPUT_LEFT:
				/* Move left */
				if (target_x > 0) target_x--;
				break;
			case INPUT_RIGHT:
				/* Move right */
				if (target_x < BOARD_WIDTH - 1) target_x++;
				break;
			case INPUT_DOWN:
				/* Move down */
				if (target_y < BOARD_HEIGHT - 1) target_y++;
				break;
			case INPUT_UP:
				/* Move up */
				if (target_y > 0) target_y--;
				break;
			case INPUT_SWEEP:
				/* Sweep cell */
				sweep_cell(board, cover, target_x, target_y, &board_initialized, &game_over);
				break;
			case INPUT_FLAG:
				/* Toggle flag */
				toggle_flag(cover, target_x, target_y, &mine_count);
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
