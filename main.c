#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define ESC_SQC_LEN 6
#define ESC_SQC "\x1b[H"

#define DEF_ROW_SIZE 16

#define ARROW_MOD1 27
#define ARROW_MOD2 91
#define ARROW_DOWN 66
#define ARROW_UP   65

// Save original terminal state
struct termios orig_termios;

typedef struct {
    char *chars;
    size_t len; // Number of characters in `chars`
    size_t mem_size; // Number of bytes reserved
} e_row;

bool row_mem_is_full(e_row *row) {
    if (row->len + 1 >= row->mem_size) {
        return true;
    }
    return false;
}

void row_mem_extend(e_row *row, size_t add_bytes) {
    row->chars = realloc(row->chars, row->mem_size + add_bytes);
}

void row_write_char(e_row *row, uint8_t idx, char add) {
    if (row_mem_is_full(row)) { row_mem_extend(row, 8); }
    row->chars[idx] = add;
    row->len++;
    row->chars[row->len] = '\0';
    fflush(stdout);
}

typedef struct {
    size_t x;
    size_t y;
} Cursor;

typedef struct {
    e_row *rows;
    size_t rows_count;
    Cursor cursor;

    int mode;
    int last_key_type;
} EditorState;

void e_append_row(EditorState *state, e_row row) {
    state->rows = realloc(state->rows, state->rows_count * sizeof(e_row) + sizeof(e_row));
    state->rows[state->rows_count] = row;
    state->rows_count++;
}

typedef enum {
    MODE_STANDARD,
    MODE_EDIT
} EditorMode;

typedef enum {
    LKT_KEY,

    LKT_ARROW_MOD1, // 27
    LKT_ARROW_MOD2, // 91
    LKT_ARROW,
} LastKeyType;

void redraw() {}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    enable_raw_mode();

    EditorState state;
    state.rows = NULL;
    state.rows_count = 0;

    Cursor cur = { .x = 0, .y = 0 };
    state.cursor = cur;

    e_row init_row;
    init_row.chars = malloc(DEF_ROW_SIZE);
    init_row.chars[0] = '\0';
    init_row.mem_size = DEF_ROW_SIZE;
    init_row.len = 0;
    e_append_row(&state, init_row);

    state.mode = MODE_EDIT;

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
        if (state.mode == MODE_STANDARD) {
            if (false) {}
        }
        else if (state.mode == MODE_EDIT) {
            // Arrow key modifier
            if (c == ARROW_MOD1) { state.last_key_type = LKT_ARROW_MOD1; continue; }
            else if (c == ARROW_MOD2 && state.last_key_type == LKT_ARROW_MOD1) { state.last_key_type = LKT_ARROW_MOD2; continue; }

            // Arrow keys
            else if (c == ARROW_DOWN && state.last_key_type == LKT_ARROW_MOD2) {
                state.cursor.y++;
                state.cursor.x = 0;
                state.last_key_type = LKT_KEY;

                // If next row does not exist, create it
                if (state.cursor.y == state.rows_count) {
                    e_row row;
                    row.chars = malloc(DEF_ROW_SIZE);
                    row.chars[0] = '\0';
                    row.mem_size = DEF_ROW_SIZE;
                    row.len = 0;

                    e_append_row(&state, row);
                }

                continue;
            }

            else if (c == ARROW_UP && state.last_key_type == LKT_ARROW_MOD2) {
                if (state.cursor.y == 0) { continue; }
                state.cursor.y--;

                // TODO: set cursor.x to end of line if previous line is shorter than cursor
                if (state.rows[state.cursor.y].len < state.cursor.x) {
                    state.cursor.x = state.rows[state.cursor.y].len;
                }

                continue;
            }

            row_write_char(&state.rows[state.cursor.y], state.cursor.x, c);
            state.cursor.x++;

            printf("char: '%c' | x: %d y: %d \n\r", c, state.cursor.x, state.cursor.y);
            fflush(stdout);
        }

        redraw(); // TODO
    }


    for (int row = 0; row < state.rows_count; row++) {
        printf("row %d: %s \n\r", row, state.rows[row].chars);
    }

    disable_raw_mode();
    return 0;
}
