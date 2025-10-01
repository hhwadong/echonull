#include "../include/ui_utils.h"
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

void clearScreen(void) {
    printf("\033[2J\033[H");
}

void setColor(const char *color) {
    printf("%s", color);
}

void resetColor(void) {
    printf("\033[0m");
}

void printCentered(const char *text, int width) {
    int len = strlen(text);
    int padding = (width - len) / 2;
    if (padding > 0) {
        printf("%*s%s%*s", padding, "", text, padding, "");
    } else {
        printf("%s", text);
    }
}

void printBoxed(const char *text, int width, const char *color) {
    int len = strlen(text);
    int padding = (width - len - 4) / 2;

    setColor(color);
    printf("┌");
    for (int i = 0; i < width - 2; i++) printf("─");
    printf("┐\n");

    printf("│");
    if (padding > 0) printf("%*s", padding, "");
    printf("%s", text);
    if (padding > 0) printf("%*s", padding, "");
    if ((len + padding * 2) < width - 4) printf(" ");
    printf("│\n");

    printf("└");
    for (int i = 0; i < width - 2; i++) printf("─");
    printf("┘\n");
    resetColor();
}

void printSeparator(int width, char c) {
    for (int i = 0; i < width; i++) {
        putchar(c);
    }
    putchar('\n');
}

int getTerminalWidth(void) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col ? w.ws_col : 80;
}

void showLoadingBar(const char *message, int duration) {
    printf("%s", message);
    fflush(stdout);

    for (int i = 0; i < duration; i++) {
        usleep(50000); // 50ms
        printf("█");
        fflush(stdout);
    }
    printf("\n\n");
}

void printSuccess(const char *message) {
    setColor(COLOR_GREEN);
    printf("✓ %s\n", message);
    resetColor();
}

void printError(const char *message) {
    setColor(COLOR_RED);
    printf("✗ %s\n", message);
    resetColor();
}

void printWarning(const char *message) {
    setColor(COLOR_YELLOW);
    printf("⚠ %s\n", message);
    resetColor();
}

void printInfo(const char *message) {
    setColor(COLOR_CYAN);
    printf("ℹ %s\n", message);
    resetColor();
}