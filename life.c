#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

#define DEFAULT_MS_FRAME_DELAY 200
#define LIVE_CELL 'o'
#define DEAD_CELL ' '

// Print the menu in which the player adds cells to the grid
void printMenu(const char* buffer, int generation, int population) {
    printw("Generation: %d Population: %d\n", generation, population);
    addstr(buffer);
    attrset(COLOR_PAIR(1));
    addstr("\n\nEnter");
    attrset(COLOR_PAIR(0));
    addstr(" Flip Cell");

    attrset(COLOR_PAIR(1));
    addstr("\nArrow Keys");
    attrset(COLOR_PAIR(0));
    addstr(" Move Around");

    attrset(COLOR_PAIR(1));
    addstr("\nSpace");
    attrset(COLOR_PAIR(0));
    addstr(" Start Simulation");

    attrset(COLOR_PAIR(1));
    addstr("\nQ");
    attrset(COLOR_PAIR(0));
    addstr(" Quit");
}

// Print the current generation of the cellular automata
void printAutomata(const char* buffer, int generation, int population) {
    printw("Generation: %d Population: %d\n", generation, population);
    addstr(buffer);
    attrset(COLOR_PAIR(1));
    addstr("\nQ");
    attrset(COLOR_PAIR(0));
    addstr(" Quit");
}

void calculateNextGeneration(char* bufferRead, char* bufferWrite, int gridWidth, size_t totalChars, int* population) {
    memcpy(bufferRead, (void*)bufferWrite, totalChars * sizeof(char));

    for (int x = 1; x < gridWidth + 1; x++) {
        for (int y = 1; y < gridWidth + 1; y++) {
            int idx = x + y * (gridWidth + 3);

            // count live neighbors
            int live_neighbors = 0;

            live_neighbors += (int)(bufferRead[idx - 1] == LIVE_CELL); // left
            live_neighbors += (int)(bufferRead[idx + 1] == LIVE_CELL); // right
            live_neighbors += (int)(bufferRead[idx - (gridWidth + 3)] == LIVE_CELL); // bottom
            live_neighbors += (int)(bufferRead[idx + (gridWidth + 3)] == LIVE_CELL); // top
            live_neighbors += (int)(bufferRead[idx - 1 - (gridWidth + 3)] == LIVE_CELL); // bottom left
            live_neighbors += (int)(bufferRead[idx - 1 + (gridWidth + 3)] == LIVE_CELL); // bottom right
            live_neighbors += (int)(bufferRead[idx + 1 - (gridWidth + 3)] == LIVE_CELL); // top left
            live_neighbors += (int)(bufferRead[idx + 1 + (gridWidth + 3)] == LIVE_CELL); // bottom right

            if (bufferRead[idx] == LIVE_CELL) {
                // if alive and has less than 2 or more than 3 neighbors it dies
                if (live_neighbors < 2 || live_neighbors > 3) {
                    bufferWrite[idx] = DEAD_CELL;
                    (*population)--;
                } 
            } else if (bufferRead[idx] == DEAD_CELL) {
                // if its dead, then if it has three live neighbors it will come back to life
                if (live_neighbors == 3) {
                    bufferWrite[idx] = LIVE_CELL;
                    (*population)++;
                }
            }
        }
    }
}

void nextFrame(char* bufferRead, char* bufferWrite, int gridWidth, size_t totalChars, int generation, int* population) {
    calculateNextGeneration(bufferRead, bufferWrite, gridWidth, totalChars, population);
    clear();
    printAutomata(bufferWrite, generation, *population);
    refresh();
}

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

    // cellular automata stats
    int generation = 0;
    int population = 0;

    char* endptr;

    // If we have a fourth argument, it will be the FPS
    if (argc == 4) {
        int fps = strtol(argv[3], &endptr, 10);
        if (*endptr != '\0') {
            fputs("Value provided for optional argument 3 - FPS - is not a valid integer\n", stderr);
            return EXIT_FAILURE;
        }
        msFrameDelay = 1000/fps;
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
    size_t totalChars = ((width + 3) * (height + 2)) + 1;

    char* bufferWrite = (char*)malloc(sizeof(char) * totalChars);

    if (bufferWrite == NULL) {
        fputs("Failed to allocate memory for write framebuffer!", stderr);
        return EXIT_FAILURE;
    }
    
    memset(bufferWrite, DEAD_CELL, sizeof(char) * totalChars);

    // TODO: grid setup is messy, must cleanup!
    // add newlines to the end of every line
    for (int i = 0; i < height + 1; i++) {
        bufferWrite[(width + 2) + i * ( width + 3)] = '\n';
    }

    // top line
    bufferWrite[0] = '+';
    for (int idx = 1; idx < width + 1; idx++) {
        bufferWrite[idx] = '-';
    }
    bufferWrite[width + 1] = '+';

    // middle
    for (int i = 1; i < height+1; i++) {
        bufferWrite[i * (width + 3)] = '|';
        bufferWrite[i * (width + 3) + width + 1] = '|';
    }

    // top line
    bufferWrite[(width + 3) * (height + 1)] = '+';
    for (int idx = 1; idx < width + 1; idx++) {
        bufferWrite[idx + ((width + 3) * (height + 1))] = '-';
    }
    bufferWrite[totalChars-3] = '+';

    // add null terminator
    bufferWrite[totalChars-1] = '\0';

    // Init curses screen
    initscr();
    if (has_colors()) {
        use_default_colors();
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_WHITE);
    }

    // Draw our char buffer and refresh the screen
    printMenu(bufferWrite, generation, population);
    refresh();

    // Our initial mouse position is (1,2) as to not be on the border
    int x = 1;
    int y = 1;
    move(y+1, x);

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
                    move(y+1,x);
                }
                break;
            }
            case KEY_RIGHT: {
                if (x < width) {
                    x += 1;
                    move(y+1,x);
                }
                break;
            }
            case KEY_UP: {
                if (y > 1) {
                    y -= 1;
                    move(y+1,x);
                }
                break;
            }
            case KEY_DOWN: {
                if (y < height) {
                    y += 1;
                    move(y+1,x);
                }
                break;
            }
            case 10: {
                int cellIdx = y * (width + 3) + x;
                if (bufferWrite[cellIdx] == LIVE_CELL) {
                    bufferWrite[cellIdx] = DEAD_CELL;
                    population--;
                }
                else if (bufferWrite[cellIdx] == DEAD_CELL) {
                    bufferWrite[cellIdx] = LIVE_CELL;
                    population++;
                }
                clear();
                printMenu(bufferWrite, generation, population);
                refresh();
                move(y+1,x);
                break;
            }
            case 'q': {
                endwin();
                free(bufferWrite);
                return EXIT_SUCCESS;
            }
        }
    } while (inputCode != ' ');

    // Start simulation
    curs_set(0);
    // we will need to copy the buffer for reading in the for loop, so we allocate space for that one too
    char* bufferRead = (char*)malloc(sizeof(char) * totalChars);

    // if we fail to allocate memory for the read framebuffer, free the write framebuffer stop ncurses and return
    if (bufferRead == NULL) {
        fputs("Failed to allocate memory for read framebuffer!", stderr);
        free(bufferWrite);
        endwin();
        return EXIT_FAILURE;
    }
    
    nextFrame(bufferRead, bufferWrite, width, totalChars, generation, &population);
    timeout(msFrameDelay);

    do {
        inputCode = getch();
        generation++;
        nextFrame(bufferRead, bufferWrite, width, totalChars, generation, &population);
    } while (inputCode != 'q');

    // Cleanup
    endwin();
    free(bufferRead);
    free(bufferWrite);
    return EXIT_SUCCESS;
}
