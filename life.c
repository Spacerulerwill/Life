#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    /*
    Program only takes 2 arguments, the width and height of the grid
    so if we receive any number other than these 3 arguments:
    * Name of program (implicitly passed)
    * Width (int)
    * Height (int)
    */ 
    if (argc != 3) {
        fputs("Invalid number of arguments passed! Program usage: ./life (width 1 - 255) (height 1 - 255)\n", stderr);
        return EXIT_FAILURE;
    }

    // Width and height of the playable area in game of life
    uint8_t width;
    uint8_t height;

    char* endptr;
    {
        int lwidth = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0') {
            fputs("Value provided for argument 1 - width - is not a valid integer\n", stderr);
            return EXIT_FAILURE;
        }
        if (lwidth < 1 || lwidth > 255) {
            fputs("Value provided for argument 1 - width - is not in the range 1 - 255\n", stderr);
            return EXIT_FAILURE;
        }
        width = (uint8_t)lwidth;
    }

    {
        int lheight = strtol(argv[2], &endptr, 10);
        if (*endptr != '\0') {
            fputs("Value provided for argument 2 - height - is not a valid integer\n", stderr);
            return EXIT_FAILURE;
        }

        if (lheight < 1 || lheight > 255) {
            fputs("Value provided for argument 2 - height - is not in the range 1 - 255\n", stderr);
            return EXIT_FAILURE;
        }

        height = (uint8_t)lheight;
    }

    /*
    The char buffer needs to store an extra column on the left for a border
    and 2 extra column on the right for a border and then newline chars.
    There must also be an extra row on the top and bottom for borders above
    and below.
    */
    uintmax_t totalChars = (width + 3) * (height + 2);

    char* buffer = (char*)malloc(sizeof(char) * ((width + 3) * (height + 2)));
    memset(buffer, ' ', sizeof(char) * totalChars);

    // Create top border
    int idx = 0;
    buffer[idx] = '+';
    for (++idx; idx < width + 1; idx++) {
        buffer[idx] = '-';
    }
    buffer[idx++] = '+';
    buffer[idx++] = '\n';

    // For each middle row, add a border char to each side and a newline on the right
    idx = width + 3;
    for (; idx < totalChars; idx += width + 3) {
        buffer[idx] = '|';
        buffer[idx + width + 1] = '|';
        buffer[idx + width + 2] = '\n';
    }

    // Create bottom border
    idx = ((width + 3) * (height + 1));
    buffer[idx] = '+';
    for (++idx; idx < (width + 3) * (height + 1) + width + 1; idx++) {
        buffer[idx] = '-';
    }
    buffer[idx++] = '+';
    
    // Init curses screen
    initscr();

    // Draw our char buffer and refresh the screen
    addstr(buffer);
    refresh();

    // Our initial mouse position is (1,1) as to not be on the border
    int x = 1;
    int y = 1;
    move(y, x);

    // Ncurses input settings
    raw(); 
    keypad(stdscr, TRUE); 
    intrflush(stdscr, FALSE);
    noecho();

    int inputCode;
    do {
        inputCode = getch();

        switch (inputCode) {
            case KEY_LEFT: {
                if (x > 1) {
                    x -= 1;
                    move(y,x);
                }
                break;
            }
            case KEY_RIGHT: {
                if (x < width) {
                    x += 1;
                    move(y,x);
                }
                break;
            }
            case KEY_UP: {
                if (y > 1) {
                    y -= 1;
                    move(y,x);
                }
                break;
            }
            case KEY_DOWN: {
                if (y < height) {
                    y += 1;
                    move(y,x);
                }
                break;
            }
            case 10: {
                buffer[(y * (width + 3)) + x] = 'o';
                clear();
                addstr(buffer);
                refresh();
                move(y,x);
                break;
            }
        }
    } while (inputCode != ' ');

    // Start simulation
    curs_set(0);
    timeout(1000);
    
    do {
        inputCode = getch();

        for (int i = 0; i < totalChars; i++) {
            if (buffer[i] == 'o') {
                buffer[i] = ' ';
            } 
            else if (buffer[i] == ' ') {
                buffer[i] = 'o';
            }
        }

        clear();
        addstr(buffer);
        refresh();
    } while (inputCode != 'q');

    // Cleanup
    endwin();
    free(buffer);
    return EXIT_SUCCESS;
}