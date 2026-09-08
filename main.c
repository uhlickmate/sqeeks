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

// Save original terminal state
struct termios orig_termios;

typedef struct {
    char *chars;
    size_t len;
    size_t capactity;
} e_row;

bool row_has_space(e_row *row) {
    
}

void row_extend_mem(e_row *row) {

}

void row_append_char(e_row *row, char add) {
    row->chars[row->len] = add;
    row->len++;
    printf("%s \n\r", row->chars);
    fflush(stdout);
}

typedef struct {
    e_row *rows;
} EditorState;

void redraw() {
    
}

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
    state.rows = malloc(sizeof(e_row));

    state.rows[0].chars = malloc(16);
    state.rows[0].chars[0] = '\0';
    state.rows[0].len = 0;

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {

        row_append_char(&state.rows[0], c);

        // printf("%s", state.rows[0].chars);
        // fflush(stdout);

        redraw();
    }

    disable_raw_mode();
    return 0;
}
