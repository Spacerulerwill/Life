#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

#define DEFAULT_MS_FRAME_DELAY 200

int main(int argc, char *argv[])
{
    /*
    Program takes 3 arguments, the width and height of the grid
    and an optional argument for the FPS so if we recieve 3 arguments
    we interpret it as only width and height
    */ 
    if (argc != 3 && argc != 4) {
        fputs("Invalid number of arguments passed! Program usage: ./life (width: int 1 - 255) (height: int 1 - 255) OPTIONAL (FPS: int)\n", stderr);
        return EXIT_FAILURE;
    }

    // Width and height of the playable area in game of life
    uint8_t width;
    uint8_t height;
    int msFrameDelay;

    char* endptr;

    // If we have a fourth argument, it will be the FPS
    if (argc == 4) {
        int fps = strtol(argv[3], &endptr, 10);
        if (*endptr != '\0') {
            fputs("Value provided for optional argument 3 - FPS - is not a valid integer\n", stderr);
            return EXIT_FAILURE;
        }
        msFrameDelay = (int)(1.0f / (float)fps);
    } else {
        msFrameDelay = DEFAULT_MS_FRAME_DELAY;
    }

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
    and below. We also need one extra char for null terminator!
    */
    uintmax_t totalChars = ((width + 3) * (height + 2));

    char* buffer = (char*)malloc(sizeof(char) * totalChars);
    memset(buffer, ' ', sizeof(char) * totalChars);

    // we will need to copy the buffer in the for loop, so we allocate space for that one too
    char* bufferCopy = (char*)malloc(sizeof(char) * totalChars);

    // TODO: grid setup is messy, must cleanup!

    // add newlines to the end of every line
    for (int i = 0; i < height + 2; i++) {
        buffer[(width + 2) + i * ( width + 3)] = '\n';
    }

    // top line
    buffer[0] = '+';
    for (int idx = 1; idx < width + 1; idx++) {
        buffer[idx] = '-';
    }
    buffer[width + 1] = '+';

    // middle
    for (int i = 1; i < height+1; i++) {
        buffer[i * (width + 3)] = '|';
        buffer[i * (width + 3) + width + 1] = '|';
    }

    // top line
    buffer[(width + 3) * (height + 1)] = '+';
    for (int idx = 1; idx < width + 1; idx++) {
        buffer[idx + ((width + 3) * (height + 1))] = '-';
    }
    buffer[totalChars-2] = '+';

    // add null terminator
    buffer[totalChars-1] = '\0';

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
                int cellIdx = y * (width + 3) + x;
                if (buffer[cellIdx] == 'o') {
                    buffer[cellIdx] = ' ';
                }
                else if (buffer[cellIdx] == ' ') {
                    buffer[cellIdx] = 'o';
                }
                clear();
                addstr(buffer);
                refresh();
                move(y,x);
                break;
            }
            case 'q': {
                goto exit;
            }
        }
    } while (inputCode != ' ');

    // Start simulation
    curs_set(0);
    timeout(msFrameDelay);

    do {
        inputCode = getch();
        memcpy(bufferCopy, (void*)buffer, totalChars * sizeof(char));

        for (int x = 1; x < width + 1; x++) {
            for (int y = 1; y < width + 1; y++) {
                int idx = x + y * (width + 3);

                // count live neighbors
                int live_neighbors = 0;

                live_neighbors += (int)(bufferCopy[idx - 1] == 'o'); // left
                live_neighbors += (int)(bufferCopy[idx + 1] == 'o'); // right
                live_neighbors += (int)(bufferCopy[idx - (width + 3)] == 'o'); // bottom
                live_neighbors += (int)(bufferCopy[idx + (width + 3)] == 'o'); // top
                live_neighbors += (int)(bufferCopy[idx - 1 - (width + 3)] == 'o'); // bottom left
                live_neighbors += (int)(bufferCopy[idx - 1 + (width + 3)] == 'o'); // bottom right
                live_neighbors += (int)(bufferCopy[idx + 1 - (width + 3)] == 'o'); // top left
                live_neighbors += (int)(bufferCopy[idx + 1 + (width + 3)] == 'o'); // bottom right

                if (bufferCopy[idx] == 'o') {
                    // if alive and has less than 2 or more than 3 neighbors it dies
                    if (live_neighbors < 2 || live_neighbors > 3) {
                        buffer[idx] = ' ';
                    } 
                } else if (bufferCopy[idx] == ' ') {
                    // if its dead, then if it has three live neighbors it will come back to life
                    if (live_neighbors == 3) {
                        buffer[idx] = 'o';
                    }
                }
            }
        }

        clear();
        addstr(buffer);
        refresh();
    } while (inputCode != 'q');

    // Cleanup
exit:  
    endwin();
    free(buffer);
    free(bufferCopy);
    return EXIT_SUCCESS;
}