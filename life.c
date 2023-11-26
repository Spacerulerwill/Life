#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        return EXIT_FAILURE;
    }

    uintmax_t width = strtoumax(argv[1], NULL, 10);
    if ((width == UINTMAX_MAX && errno == ERANGE) || width == 0) {
        return EXIT_FAILURE;
    }

    uintmax_t height = strtoumax(argv[2], NULL, 10);
    if ((height == UINTMAX_MAX && errno == ERANGE) || height == 0) {
        return EXIT_FAILURE;
    }

    uintmax_t totalChars = (width + 3) * (height + 2);

    char* buffer = (char*)malloc(sizeof(char) * ((width + 3) * (height + 2)));
    memset(buffer, ' ', sizeof(char) * totalChars);

    // Create grid in char buffer
    // top line
    int idx = 0;
    buffer[idx] = '+';
    for (++idx; idx < width + 1; idx++) {
        buffer[idx] = '-';
    }
    buffer[idx++] = '+';
    buffer[idx++] = '\n';

    // middle lines
    idx = width + 3;
    for (; idx < totalChars; idx += width + 3) {
        buffer[idx] = '|';
        buffer[idx + width + 1] = '|';
        buffer[idx + width + 2] = '\n';
    }

    // bottom line
    idx = ((width + 3) * (height + 1));
    buffer[idx] = '+';
    for (++idx; idx < (width + 3) * (height + 1) + width + 1; idx++) {
        buffer[idx] = '-';
    }
    buffer[idx++] = '+';

    initscr();
    addstr(buffer);
    refresh();
    int inputCode;
    int x = 1;
    int y = 1;
    move(y, x);
    raw(); 
    keypad(stdscr, TRUE); 
    intrflush(stdscr, FALSE);
    noecho();
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
    } while (inputCode != 'q');
    endwin();
    free(buffer);
    return EXIT_SUCCESS;
}