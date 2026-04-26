#include "gui.h"
#include "compressor.h"
#include "dynbuf.h"
#include "ratio.h"

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
#include <ncurses/ncurses.h>
#include <stdlib.h>
#include <string.h>

#define LOG_INIT_CAP  64
#define LOG_LINE_MAX  256

static DynBuf           log_buf;
static pthread_mutex_t  log_lock = PTHREAD_MUTEX_INITIALIZER;

static WINDOW *win_title    = NULL;
static WINDOW *win_stats    = NULL;
static WINDOW *win_bar      = NULL;
static WINDOW *win_log      = NULL;
static WINDOW *win_controls = NULL;

#define TITLE_H     3
#define STATS_H     6
#define BAR_H       12
#define CONTROLS_H  3


static void log_append(const char *fmt, ...) {
    char line[LOG_LINE_MAX];

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int prefix_len = snprintf(line, LOG_LINE_MAX, "[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line + prefix_len, LOG_LINE_MAX - prefix_len, fmt, ap);
    va_end(ap);

    pthread_mutex_lock(&log_lock);
    dynbuf_append(&log_buf, line);
    pthread_mutex_unlock(&log_lock);
}

static void log_flush_to_file(const char *path) {
    pthread_mutex_lock(&log_lock);
    FILE *f = fopen(path, "w");
    if (f) {
        for (int i = 0; i < log_buf.count; i++)
            fprintf(f, "%s\n", log_buf.lines[i]);
        fclose(f);
    }
    pthread_mutex_unlock(&log_lock);
}

/* ─── Window Layout ──────────────────────────────────────────────────── */

static void rebuild_windows(void) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    if (win_title)    { delwin(win_title);    win_title    = NULL; }
    if (win_stats)    { delwin(win_stats);    win_stats    = NULL; }
    if (win_bar)      { delwin(win_bar);      win_bar      = NULL; }
    if (win_log)      { delwin(win_log);      win_log      = NULL; }
    if (win_controls) { delwin(win_controls); win_controls = NULL; }

    int log_h = rows - TITLE_H - STATS_H - BAR_H - CONTROLS_H;
    if (log_h < 3) log_h = 3;

    int y = 0;
    win_title    = newwin(TITLE_H,    cols, y, 0); y += TITLE_H;
    win_stats    = newwin(STATS_H,    cols, y, 0); y += STATS_H;
    win_bar      = newwin(BAR_H,      cols, y, 0); y += BAR_H;
    win_log      = newwin(log_h,      cols, y, 0); y += log_h;
    win_controls = newwin(CONTROLS_H, cols, y, 0);

    scrollok(win_log, TRUE);
    clear();
    refresh();
}

/* ─── Drawing ────────────────────────────────────────────────────────── */

static void draw_title(void) {
    int cols = getmaxx(win_title);
    wbkgd(win_title, COLOR_PAIR(1));
    werase(win_title);
    const char *title = " Multi-Threaded File Compression System ";
    int x = (cols - (int)strlen(title)) / 2;
    if (x < 0) x = 0;
    mvwprintw(win_title, 1, x, "%s", title);
    wrefresh(win_title);
}

static void draw_stats(int completed, int total, const RatioReport *r) {
    werase(win_stats);
    box(win_stats, 0, 0);
    mvwprintw(win_stats, 0, 2, " Statistics ");
    mvwprintw(win_stats, 1, 2, "Chunks    : %d / %d", completed, total);
    mvwprintw(win_stats, 2, 2, "Original  : %.2f KB", r->original_kb);
    mvwprintw(win_stats, 3, 2, "Compressed: %.2f KB", r->compressed_kb);
    wattron(win_stats, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(win_stats, 4, 2, "Ratio     : %.1f%% space saved", r->percent_saved);
    wattroff(win_stats, COLOR_PAIR(2) | A_BOLD);
    wrefresh(win_stats);
}

static void draw_progress_bar(int completed, int total, const ProgressStats *s) {
    int cols = getmaxx(win_bar);
    int pct  = (total > 0) ? (completed * 100 / total) : 0;

    werase(win_bar);
    box(win_bar, 0, 0);
    mvwprintw(win_bar, 0, 2, " CPU Slots & Progress ");

    for (int i = 0; i < 8; i++) {
        mvwprintw(win_bar, i + 1, 2, "Slot %d: [", i + 1);
        int active_id = s->active_slots[i];
        if (active_id > 0) {
            wattron(win_bar, COLOR_PAIR(4) | A_BOLD);
            mvwprintw(win_bar, i + 1, 11, "BUSY - Chunk %03d", active_id - 1);
            wattroff(win_bar, COLOR_PAIR(4) | A_BOLD);
        } else {
            mvwprintw(win_bar, i + 1, 11, "IDLE             ");
        }
        mvwprintw(win_bar, i + 1, 29, "]");
    }

    int bar_width = cols - 12;
    if (bar_width < 4) bar_width = 4;
    int filled = (bar_width * pct) / 100;

    mvwprintw(win_bar, 10, 2, "[");
    wattron(win_bar, COLOR_PAIR(3) | A_REVERSE);
    for (int i = 0; i < filled; i++) waddch(win_bar, ' ');
    wattroff(win_bar, COLOR_PAIR(3) | A_REVERSE);
    for (int i = filled; i < bar_width; i++) waddch(win_bar, '-');
    mvwprintw(win_bar, 10, cols - 8, "] %3d%%", pct);

    wrefresh(win_bar);
}

static void draw_log(void) {
    int log_rows = getmaxy(win_log) - 2;
    int log_cols = getmaxx(win_log) - 4;

    werase(win_log);
    box(win_log, 0, 0);
    mvwprintw(win_log, 0, 2, " Log ");

    pthread_mutex_lock(&log_lock);
    int start = log_buf.count - log_rows;
    if (start < 0) start = 0;
    for (int i = start; i < log_buf.count; i++) {
        char line[LOG_LINE_MAX];
        strncpy(line, log_buf.lines[i], LOG_LINE_MAX - 1);
        line[LOG_LINE_MAX - 1] = '\0';
        if ((int)strlen(line) > log_cols) line[log_cols] = '\0';
        wattron(win_log, COLOR_PAIR(4));
        mvwprintw(win_log, 1 + (i - start), 2, "%s", line);
        wattroff(win_log, COLOR_PAIR(4));
    }
    pthread_mutex_unlock(&log_lock);
    wrefresh(win_log);
}

static void draw_controls(void) {
    werase(win_controls);
    box(win_controls, 0, 0);
    mvwprintw(win_controls, 1, 2, "[Q] Quit   [R] Resize");
    wrefresh(win_controls);
}

void gui_init() {
    dynbuf_init(&log_buf, LOG_INIT_CAP);

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(0);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_CYAN);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_BLACK, COLOR_GREEN);
        init_pair(4, COLOR_CYAN,  COLOR_BLACK);
    }

    rebuild_windows();
    log_append("GUI initialised.");
}

void gui_update(int completed, int total) {
    ProgressStats s = compressor_get_stats();
    RatioReport   r = ratio_compute(s.total_original_bytes,
                                    s.total_compressed_bytes);

    log_append("Chunk %d / %d compressed.", completed, total);

    draw_title();
    draw_stats(completed, total, &r);
    draw_progress_bar(completed, total, &s);
    draw_log();
    draw_controls();

    int ch = getch();
    switch (ch) {
        case 'q': case 'Q':
            log_append("User requested quit.");
            gui_destroy();
            exit(0);
            break;
        case KEY_RESIZE:
        case 'r': case 'R':
            rebuild_windows();
            log_append("Window resized.");
            break;
        default:
            break;
    }
}

void gui_destroy() {
    if (win_title)    delwin(win_title);
    if (win_stats)    delwin(win_stats);
    if (win_bar)      delwin(win_bar);
    if (win_log)      delwin(win_log);
    if (win_controls) delwin(win_controls);
    endwin();
    log_flush_to_file("compression.log");
    pthread_mutex_lock(&log_lock);
    dynbuf_free(&log_buf);
    pthread_mutex_unlock(&log_lock);
}