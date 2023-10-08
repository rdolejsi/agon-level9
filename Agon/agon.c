/***********************************************************************\
*
* Level 9 interpreter
* Version 5.2
* Copyright (c) 1996-2023 Glen Summers and contributors.
* Contributions from David Kinder, Alan Staniforth, Simon Baldwin,
* Dieter Baron and Andreas Scherrer.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
*
\***********************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "level9.h"

#define TEXTBUFFER_SIZE 10240
char text_buffer[TEXTBUFFER_SIZE + 1];
int text_buffer_pointer = 0;

uint8_t column = 0;
static uint16_t screen_width;
static uint16_t screen_height;
static uint8_t screen_columns;
static uint8_t screen_rows;
static uint8_t screen_colors;

/**
 * Loads the current graphic mode details for future use.
 */
void os_graphics_inquiry() {
    screen_width = getsysvar_scrwidth();
    screen_height = getsysvar_scrheight();
    screen_columns = getsysvar_scrCols();
    screen_rows = getsysvar_scrRows();
    screen_colors = getsysvar_scrColours();
}

void os_printchar(char c) {
    if (c == '\r') {
        os_flush();
        putchar('\n');
        column = 0;
    } else if (isprint(c) != 0) {
        if (text_buffer_pointer >= TEXTBUFFER_SIZE) {
            os_flush();
        }
        *(text_buffer + (text_buffer_pointer++)) = c;
    }
}

L9BOOL os_input(char *ibuff, int size) {
    os_flush();
    fgets(ibuff, size, stdin);
    char *nl = strchr(ibuff, '\n');
    if (nl) {
        *nl = 0;
    }
    return TRUE;
}

char os_readchar(int millis) {
    os_flush();
    if (millis == 0) {
        return 0;
    }

    /* Some of the Level 9 games expect to be able to wait for
       a character for a short while as a way of pausing, and
       expect 0 to be returned, while the multiple-choice games
       (such as The Archers) expect 'proper' keys from this
       routine.

       To get round this, we return 0 for the first 1024 calls,
       and 'proper' keys thereafter. Since The Archers and
       similar games ignore the returned zeros, this works quite
       well. A 'correct' port would solve this properly by
       implementing a timed wait for a key, but this is not
       possible using only C stdio-functions.
    */
    static int count = 0;
    if (++count < 1024) {
        return 0;
    }
    count = 0;

    char c = getc(stdin); /* will require enter key as well */
    if (c != '\n') {
        while (getc(stdin) != '\n') {
            /* remove input from buffer until enter key */
        }
    }
    return c;
}

L9BOOL os_stoplist(void) {
    return FALSE;
}

void os_flush(void) {
    if (text_buffer_pointer < 1) {
        return;
    }
    *(text_buffer + text_buffer_pointer) = ' ';
    int ptr = 0;
    while (text_buffer_pointer + column > screen_columns - 1) {
        register int space = ptr;
        register int last_space = space;
        register bool searching = TRUE;
        while (searching) {
            while (text_buffer[space] != ' ') {
                space++;
            }
            if (space - ptr + column > screen_columns - 1) {
                space = last_space;
                printf("%.*s\n", space - ptr, text_buffer + ptr);
                column = 0;
                space++;
                if (text_buffer[space] == ' ') {
                    space++;
                }
                text_buffer_pointer -= (space - ptr);
                ptr = space;
                searching = FALSE;
            } else {
                last_space = space;
            }
            space++;
        }
    }
    if (text_buffer_pointer > 0) {
        printf("%.*s", text_buffer_pointer, text_buffer + ptr);
        column += text_buffer_pointer;
    }
    text_buffer_pointer = 0;
}

FILE *os_prompt_and_open_file(const char *prompt, const char *mode) {
    char name[256];
    os_flush();
    printf("%s", prompt);
    fgets(name, 256, stdin);
    char *nl = strchr(name, '\n');
    if (nl) {
        *nl = 0;
    }
    return fopen(name, mode);
}

L9BOOL os_save_file(L9BYTE *Ptr, int Bytes) {
    FILE *f = os_prompt_and_open_file("Save file: ", "wb");
    if (!f) {
        return FALSE;
    }
    fwrite(Ptr, 1, Bytes, f);
    fclose(f);
    return TRUE;
}

L9BOOL os_load_file(L9BYTE *pointer, int *bytes, int max) {
    FILE *f = os_prompt_and_open_file("Load file: ", "rb");
    if (!f) {
        return FALSE;
    }
    *bytes = fread(pointer, 1, max, f);
    fclose(f);
    return TRUE;
}

L9BOOL os_get_game_file(char *new_name, int size) {
    os_flush();
    printf("Load next game: ");
    fgets(new_name, size, stdin);
    char *nl = strchr(new_name, '\n');
    if (nl) {
        *nl = 0;
    }
    return TRUE;
}

void os_set_filenumber(char *new_name, int size, int n) {
    char *p = strrchr(new_name, '/');
    if (p == NULL) {
        p = new_name;
    }
    for (int i = strlen(p) - 1; i >= 0; i--) {
        if (isdigit(p[i])) {
            p[i] = '0' + n;
            return;
        }
    }
}

FILE *os_open_script_file(void) {
    return os_prompt_and_open_file("Script file: ", "rt");
}

L9BOOL os_find_file(char *new_name) {
    FILE *f = fopen(new_name, "rb");
    if (f != NULL) {
        fclose(f);
        return TRUE;
    }
    return FALSE;
}

void os_graphics(int mode) {
}

void os_cleargraphics(void) {
}

void os_setcolour(int colour, int index) {
}

void os_drawline(int x1, int y1, int x2, int y2, int colour1, int colour2) {
}

void os_fill(int x, int y, int colour1, int colour2) {
}

void os_show_bitmap(int pic, int x, int y) {
}

int main(int argc, char **argv) {
    os_graphics_inquiry();
    printf(
            "Level 9 Interpreter, mode %ix%i (%ix%i chars), %i colors\n",
            screen_width, screen_height, screen_columns, screen_rows, screen_colors
    );
    if (argc != 2) {
        printf("Use: %s <game_file>\n", argv[0]);
        return 0;
    }
    if (!LoadGame(argv[1], NULL)) {
        printf("Error: Unable to open game file\n");
        return 0;
    }
    while (RunGame());
    StopGame();
    FreeMemory();
    return 0;
}
