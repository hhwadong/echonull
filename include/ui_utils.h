#ifndef UI_UTILS_H
#define UI_UTILS_H

#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_GRAY    "\033[90m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_RESET   "\033[0m"

void clearScreen(void);
void setColor(const char *color);
void resetColor(void);
void printCentered(const char *text, int width);
void printBoxed(const char *text, int width, const char *color);
void printSeparator(int width, char c);
int getTerminalWidth(void);
void showLoadingBar(const char *message, int duration);
void printSuccess(const char *message);
void printError(const char *message);
void printWarning(const char *message);
void printInfo(const char *message);

#endif