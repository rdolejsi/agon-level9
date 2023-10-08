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

#define GFX_FILE_EXT_DOS_HIRES ".hrc"
#define GFX_FILE_EXT_DOS_CGA ".cga"
#define GFX_FILE_DEFAULT "picture.dat"

// Graphics is not enabled at the moment as it is not supported yet.
// Not only the graphical methods will need to be implemented, but also getchar needs to be redesigned
// to not wait for next character, but just check the key pressed (to allow graphics ticking).
#undef GFX_ENABLED

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
#ifdef GFX_ENABLED
    RunGraphics();
#endif
    fgets(ibuff, size, stdin);
    char *nl = strchr(ibuff, '\n');
    if (nl) {
        *nl = 0;
    }
    return TRUE;
}

char os_readchar(int millis) {
    os_flush();
#ifdef GFX_ENABLED
    RunGraphics();
#endif
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

L9BOOL os_find_file(char *file_name) {
    FILE *f = fopen(file_name, "rb");
    if (f != NULL) {
        fclose(f);
        return TRUE;
    }
    return FALSE;
}

void os_graphics(int mode) {
#ifdef GFX_DEBUG
    printf("os_graphics(mode=%i);\n", mode);
#endif
}

void os_cleargraphics(void) {
#ifdef GFX_DEBUG
    printf("os_cleargraphics();\n");
#endif
}

void os_setcolour(int color, int index) {
#ifdef GFX_DEBUG
    printf("os_setcolour(color=%i,index=%i);\n", color, index);
#endif
}

void os_drawline(int x1, int y1, int x2, int y2, int color1, int color2) {
#ifdef GFX_DEBUG
    printf("os_drawline(x1=%i,y1=%i,x2=%i,y2=%i,color1=%i,color2=%i);\n", x1, y1, x2, y2, color1, color2);
#endif
}

void os_fill(int x, int y, int color1, int color2) {
#ifdef GFX_DEBUG
    printf("os_fill(x=%i,y=%i,color1=%i,color2=%i);\n", x, y, color1, color2);
#endif
}

void os_show_bitmap(int pic, int x, int y) {
#ifdef GFX_DEBUG
    printf("os_show_bitmap(pic=%i,x=%i,y=%i);\n", pic, x, y);
#endif
}

char *os_strip_file_ext(char *file) {
    char *ext_pos = strrchr(file, '.');
    return ext_pos != NULL ? strndup(file, strlen(file) - strlen(ext_pos)) : file;
}

/**
 * Performs a secondary graphics file lookup.
 *
 * We currently support DOS-based games coming with *.cga or *.hrc graphics,
 * where Hi-Res graphics takes precedence unless number of screen colors or resolution is below 512.
 * The graphics file can be forced from command line, though - irrespective of the screen limitations.
 *
 * @return either gfx file forced from command line or detected file based on screen preference
 */
void os_lookup_gfx_file(char *dat_file, char *gfx_file_forced, char *gfx_file) {
    if (gfx_file_forced != NULL) {
        strcpy(gfx_file, gfx_file_forced);
        return;
    }
    printf("No gfx file specified, detecting..\n");
    char *file_basename = os_strip_file_ext(dat_file);

    char dos_hires[256];
    sprintf(dos_hires, "%s%s", file_basename, GFX_FILE_EXT_DOS_HIRES);
    L9BOOL dos_hires_exists = os_find_file(dos_hires);

    char dos_cga[256];
    sprintf(dos_cga, "%s%s", file_basename, GFX_FILE_EXT_DOS_CGA);
    L9BOOL dos_cga_exists = os_find_file(dos_cga);

    L9BOOL default_exists = os_find_file(GFX_FILE_DEFAULT);

    // no gfx file detected
    gfx_file[0] = '\0'; // empty file
    if (!dos_cga_exists && !dos_hires_exists && !default_exists) {
        printf("No gfx file exists.\n");
        return;
    }
    // default file detected
    if (!dos_cga_exists && !dos_hires_exists) {
        printf("Using default gfx file %s (no other variants available).\n", GFX_FILE_DEFAULT);
        strcpy(gfx_file, GFX_FILE_DEFAULT);
        return;
    }
    // hires for good screen conditions or if cga does not exist
    if (dos_hires_exists && ((screen_width >= 512 && screen_colors >= 16) || !dos_cga_exists)) {
        printf("Using dos-hi-res gfx file %s.\n", GFX_FILE_DEFAULT);
        strcpy(gfx_file, dos_hires);
        return;
    }
    // CGA variant for everything else (320 width screen, 4 colors, ..)
    printf("Using dos-cga gfx file %s.\n", dos_cga);
    strcpy(gfx_file, dos_cga);
}

int main(int argc, char **argv) {
    os_graphics_inquiry();
    printf(
            "Level 9 Interpreter, mode %ix%i (%ix%i chars), %i colors\n",
            screen_width, screen_height, screen_columns, screen_rows, screen_colors
    );
    if (argc != 2 && argc != 3) {
        printf("Use: %s <game_file> [gfx_file]\n", argv[0]);
        return 0;
    }
    char gfx_file[256] = "";
    os_lookup_gfx_file(argv[1], argc < 3 ? NULL : argv[2], gfx_file);
    if (strlen(gfx_file) == 0) {
        printf("Loading game file %s\n", argv[1]);
    } else {
        printf("Loading game file %s + gfx file %s\n", argv[1], gfx_file);
    }
    if (!LoadGame(argv[1], gfx_file)) {
        printf("Error: Unable to open game\n");
        return 0;
    }
    while (RunGame());
    StopGame();
    FreeMemory();
    return 0;
}
