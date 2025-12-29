#include "tui/Screen.hpp"
#include <algorithm>
#include <cstring>

namespace easytty {
namespace tui {

std::unique_ptr<Screen> gScreen = nullptr;

Screen::Screen() : width_(0), height_(0), initialized_(false) {}

Screen::~Screen() {
    if (initialized_) {
        cleanup();
    }
}

void Screen::init() {
    if (initialized_) return;
    
    // Initialize ncurses
    initscr();
    
    // Setup terminal modes
    cbreak();               // Disable line buffering
    noecho();               // Don't echo input
    keypad(stdscr, TRUE);   // Enable function keys
    curs_set(0);            // Hide cursor
    
    // Initialize colors if supported
    if (has_colors()) {
        start_color();
        use_default_colors();
        initColors();
    }
    
    // Get initial dimensions
    updateDimensions();
    
    initialized_ = true;
}

void Screen::cleanup() {
    if (!initialized_) return;
    
    curs_set(1);    // Show cursor
    echo();         // Restore echo
    nocbreak();     // Restore line buffering
    endwin();       // End ncurses mode
    
    initialized_ = false;
}

void Screen::clear() {
    ::clear();
}

void Screen::refresh() {
    ::refresh();
}

void Screen::updateDimensions() {
    getmaxyx(stdscr, height_, width_);
}

void Screen::initColors() {
    // Define color pairs matching KConfig style
    init_pair(ColorScheme::NORMAL, COLOR_WHITE, COLOR_BLUE);
    init_pair(ColorScheme::HIGHLIGHT, COLOR_WHITE, COLOR_CYAN);
    init_pair(ColorScheme::TITLE, COLOR_YELLOW, COLOR_BLUE);
    init_pair(ColorScheme::STATUS, COLOR_BLACK, COLOR_WHITE);
    init_pair(ColorScheme::ERROR, COLOR_WHITE, COLOR_RED);
    init_pair(ColorScheme::SUCCESS, COLOR_WHITE, COLOR_GREEN);
    init_pair(ColorScheme::BORDER, COLOR_CYAN, COLOR_BLUE);
    init_pair(ColorScheme::DEVICE, COLOR_GREEN, COLOR_BLUE);
    
    // Set default background
    bkgd(COLOR_PAIR(ColorScheme::NORMAL));
}

void Screen::drawBox(int y, int x, int height, int width, int colorPair) {
    attron(COLOR_PAIR(colorPair));
    
    // Corners
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + width - 1, ACS_URCORNER);
    mvaddch(y + height - 1, x, ACS_LLCORNER);
    mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);
    
    // Horizontal lines
    for (int i = 1; i < width - 1; i++) {
        mvaddch(y, x + i, ACS_HLINE);
        mvaddch(y + height - 1, x + i, ACS_HLINE);
    }
    
    // Vertical lines
    for (int i = 1; i < height - 1; i++) {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + width - 1, ACS_VLINE);
    }
    
    attroff(COLOR_PAIR(colorPair));
}

void Screen::drawText(int y, int x, const std::string& text, int colorPair) {
    attron(COLOR_PAIR(colorPair));
    mvprintw(y, x, "%s", text.c_str());
    attroff(COLOR_PAIR(colorPair));
}

void Screen::drawCenteredText(int y, const std::string& text, int colorPair) {
    int x = (width_ - static_cast<int>(text.length())) / 2;
    if (x < 0) x = 0;
    drawText(y, x, text, colorPair);
}

void Screen::drawHLine(int y, int x, int length, int colorPair) {
    attron(COLOR_PAIR(colorPair));
    for (int i = 0; i < length; i++) {
        mvaddch(y, x + i, ACS_HLINE);
    }
    attroff(COLOR_PAIR(colorPair));
}

void Screen::drawTitleBar(const std::string& title) {
    std::string titleLine = " EasyTTY - " + title + " ";
    std::string padding(width_, ' ');
    
    attron(COLOR_PAIR(ColorScheme::TITLE) | A_BOLD);
    mvprintw(0, 0, "%s", padding.c_str());
    int x = (width_ - static_cast<int>(titleLine.length())) / 2;
    mvprintw(0, x, "%s", titleLine.c_str());
    attroff(COLOR_PAIR(ColorScheme::TITLE) | A_BOLD);
}

void Screen::drawStatusBar(const std::string& message, bool isError) {
    std::string padding(width_, ' ');
    int colorPair = isError ? ColorScheme::ERROR : ColorScheme::STATUS;
    
    attron(COLOR_PAIR(colorPair));
    mvprintw(height_ - 2, 0, "%s", padding.c_str());
    mvprintw(height_ - 2, 1, "%s", message.c_str());
    attroff(COLOR_PAIR(colorPair));
}

void Screen::drawHelpBar(const std::string& help) {
    std::string padding(width_, ' ');
    
    attron(COLOR_PAIR(ColorScheme::STATUS));
    mvprintw(height_ - 1, 0, "%s", padding.c_str());
    mvprintw(height_ - 1, 1, "%s", help.c_str());
    attroff(COLOR_PAIR(ColorScheme::STATUS));
}

int Screen::getInput() {
    return getch();
}

std::string Screen::getStringInput(int y, int x, int maxLen, const std::string& prompt) {
    echo();
    curs_set(1);
    
    char buffer[256] = {0};
    mvprintw(y, x, "%s", prompt.c_str());
    
    int inputX = x + static_cast<int>(prompt.length());
    move(y, inputX);
    
    getnstr(buffer, std::min(maxLen, 255));
    
    noecho();
    curs_set(0);
    
    return std::string(buffer);
}

bool Screen::showConfirmDialog(const std::string& title, const std::string& message) {
    int dialogWidth = std::max(static_cast<int>(message.length()) + 6, 
                               static_cast<int>(title.length()) + 6);
    dialogWidth = std::min(dialogWidth, width_ - 4);
    int dialogHeight = 7;
    
    int startY = (height_ - dialogHeight) / 2;
    int startX = (width_ - dialogWidth) / 2;
    
    // Draw dialog box
    attron(COLOR_PAIR(ColorScheme::NORMAL));
    for (int i = 0; i < dialogHeight; i++) {
        mvhline(startY + i, startX, ' ', dialogWidth);
    }
    attroff(COLOR_PAIR(ColorScheme::NORMAL));
    
    drawBox(startY, startX, dialogHeight, dialogWidth, ColorScheme::BORDER);
    
    // Draw title
    attron(COLOR_PAIR(ColorScheme::TITLE) | A_BOLD);
    mvprintw(startY, startX + 2, " %s ", title.c_str());
    attroff(COLOR_PAIR(ColorScheme::TITLE) | A_BOLD);
    
    // Draw message
    drawText(startY + 2, startX + 3, message, ColorScheme::NORMAL);
    
    // Draw buttons
    std::string buttons = "  [Y]es    [N]o  ";
    int btnX = startX + (dialogWidth - static_cast<int>(buttons.length())) / 2;
    drawText(startY + 4, btnX, buttons, ColorScheme::NORMAL);
    
    refresh();
    
    while (true) {
        int ch = getInput();
        if (ch == 'y' || ch == 'Y') {
            return true;
        } else if (ch == 'n' || ch == 'N' || ch == 27) { // ESC
            return false;
        }
    }
}

void Screen::showMessageDialog(const std::string& title, const std::string& message, bool isError) {
    int dialogWidth = std::max(static_cast<int>(message.length()) + 6, 
                               static_cast<int>(title.length()) + 6);
    dialogWidth = std::min(dialogWidth, width_ - 4);
    int dialogHeight = 6;
    
    int startY = (height_ - dialogHeight) / 2;
    int startX = (width_ - dialogWidth) / 2;
    
    int bgColor = isError ? ColorScheme::ERROR : ColorScheme::NORMAL;
    
    // Draw dialog box
    attron(COLOR_PAIR(bgColor));
    for (int i = 0; i < dialogHeight; i++) {
        mvhline(startY + i, startX, ' ', dialogWidth);
    }
    attroff(COLOR_PAIR(bgColor));
    
    drawBox(startY, startX, dialogHeight, dialogWidth, ColorScheme::BORDER);
    
    // Draw title
    attron(COLOR_PAIR(ColorScheme::TITLE) | A_BOLD);
    mvprintw(startY, startX + 2, " %s ", title.c_str());
    attroff(COLOR_PAIR(ColorScheme::TITLE) | A_BOLD);
    
    // Draw message
    drawText(startY + 2, startX + 3, message, bgColor);
    
    // Draw button
    std::string button = " [OK] ";
    int btnX = startX + (dialogWidth - static_cast<int>(button.length())) / 2;
    drawText(startY + 4, btnX, button, ColorScheme::NORMAL);
    
    refresh();
    
    // Wait for any key
    getInput();
}

std::string Screen::showInputDialog(const std::string& title, const std::string& prompt, const std::string& defaultValue) {
    int dialogWidth = std::max({
        static_cast<int>(prompt.length()) + 30,
        static_cast<int>(title.length()) + 6,
        50
    });
    dialogWidth = std::min(dialogWidth, width_ - 4);
    int dialogHeight = 7;
    
    int startY = (height_ - dialogHeight) / 2;
    int startX = (width_ - dialogWidth) / 2;
    
    // Draw dialog box
    attron(COLOR_PAIR(ColorScheme::NORMAL));
    for (int i = 0; i < dialogHeight; i++) {
        mvhline(startY + i, startX, ' ', dialogWidth);
    }
    attroff(COLOR_PAIR(ColorScheme::NORMAL));
    
    drawBox(startY, startX, dialogHeight, dialogWidth, ColorScheme::BORDER);
    
    // Draw title
    attron(COLOR_PAIR(ColorScheme::TITLE) | A_BOLD);
    mvprintw(startY, startX + 2, " %s ", title.c_str());
    attroff(COLOR_PAIR(ColorScheme::TITLE) | A_BOLD);
    
    // Draw prompt
    drawText(startY + 2, startX + 3, prompt, ColorScheme::NORMAL);
    
    // Draw input field
    int inputWidth = dialogWidth - 8;
    int inputY = startY + 4;
    int inputX = startX + 3;
    
    attron(COLOR_PAIR(ColorScheme::STATUS));
    for (int i = 0; i < inputWidth; i++) {
        mvaddch(inputY, inputX + i, ' ');
    }
    if (!defaultValue.empty()) {
        mvprintw(inputY, inputX, "%s", defaultValue.c_str());
    }
    attroff(COLOR_PAIR(ColorScheme::STATUS));
    
    refresh();
    
    // Get input
    echo();
    curs_set(1);
    
    char buffer[256] = {0};
    if (!defaultValue.empty()) {
        strncpy(buffer, defaultValue.c_str(), 255);
    }
    
    move(inputY, inputX);
    
    // Clear and get new input
    for (int i = 0; i < inputWidth; i++) {
        mvaddch(inputY, inputX + i, ' ');
    }
    move(inputY, inputX);
    
    getnstr(buffer, std::min(inputWidth - 1, 255));
    
    noecho();
    curs_set(0);
    
    return std::string(buffer);
}

} // namespace tui
} // namespace easytty
