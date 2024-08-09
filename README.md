# Minesweeper

This is a simple, terminal-based Minesweeper clone written in C. I made it because I wanted to get back into C programming and I really like Minesweeper.

## Usage

To play the game, clone this repository and compile the `minesweeper.c` file (make sure to link the ncurses library). Running the `make` command in this directory should compile it for you. Then run the compiled executable in a terminal.

## Potential Additions

Some things I would like to add if and when I have time:
- Timer
- Better characters/symbols for the board

## Bugs

- For some reason, after placing the mines on the board, sometimes a few cells in the top left corner of the board cover will be set to values that they are not supposed to be set to or even be able to be set to. I "fixed" this by setting all the cover cells to the `covered` enum value after placing the mines, but if your first sweep is around the top left corner, you may encounter some unexpected behavior. If you know what is causing this bug, please let me know because I haven't been able to figure it out and it has been bothering me.
