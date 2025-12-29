#pragma once

#include "common/Types.hpp"
#include <ncurses.h>
#include <string>
#include <functional>
#include <memory>

namespace easytty {
namespace tui {

/**
 * @brief Terminal UI manager using ncurses
 * 
 * Handles screen initialization, color setup, and basic drawing primitives
 */
class Screen {
public:
    Screen();
    ~Screen();
    
    // Prevent copying
    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;
    
    /**
     * @brief Initialize the screen
     */
    void init();
    
    /**
     * @brief Cleanup and restore terminal
     */
    void cleanup();
    
    /**
     * @brief Clear the screen
     */
    void clear();
    
    /**
     * @brief Refresh the screen
     */
    void refresh();
    
    /**
     * @brief Get screen dimensions
     */
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    
    /**
     * @brief Update dimensions (call after resize)
     */
    void updateDimensions();
    
    /**
     * @brief Draw a box
     */
    void drawBox(int y, int x, int height, int width, int colorPair = ColorScheme::BORDER);
    
    /**
     * @brief Draw text at position
     */
    void drawText(int y, int x, const std::string& text, int colorPair = ColorScheme::NORMAL);
    
    /**
     * @brief Draw centered text
     */
    void drawCenteredText(int y, const std::string& text, int colorPair = ColorScheme::NORMAL);
    
    /**
     * @brief Draw a horizontal line
     */
    void drawHLine(int y, int x, int length, int colorPair = ColorScheme::BORDER);
    
    /**
     * @brief Draw title bar
     */
    void drawTitleBar(const std::string& title);
    
    /**
     * @brief Draw status bar
     */
    void drawStatusBar(const std::string& message, bool isError = false);
    
    /**
     * @brief Draw help bar at bottom
     */
    void drawHelpBar(const std::string& help);
    
    /**
     * @brief Get user input (single character)
     */
    int getInput();
    
    /**
     * @brief Get string input
     */
    std::string getStringInput(int y, int x, int maxLen, const std::string& prompt);
    
    /**
     * @brief Show yes/no dialog
     */
    bool showConfirmDialog(const std::string& title, const std::string& message);
    
    /**
     * @brief Show message dialog
     */
    void showMessageDialog(const std::string& title, const std::string& message, bool isError = false);
    
    /**
     * @brief Show input dialog
     */
    std::string showInputDialog(const std::string& title, const std::string& prompt, const std::string& defaultValue = "");

private:
    int width_;
    int height_;
    bool initialized_;
    
    /**
     * @brief Initialize color pairs
     */
    void initColors();
};

// Global screen instance
extern std::unique_ptr<Screen> gScreen;

} // namespace tui
} // namespace easytty
