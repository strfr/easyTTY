#include "tui/Menu.hpp"
#include "tui/Screen.hpp"
#include <algorithm>

namespace easytty {
namespace tui {

Menu::Menu(const std::string& title, const std::string& subtitle)
    : title_(title)
    , subtitle_(subtitle)
    , selectedIndex_(0)
    , scrollOffset_(0)
    , statusIsError_(false)
    , helpText_("↑/↓: Navigate  Enter: Select  Q: Quit  ESC: Back")
    , running_(false) {}

void Menu::addItem(const MenuItem& item) {
    items_.push_back(item);
}

void Menu::addItems(const std::vector<MenuItem>& items) {
    for (const auto& item : items) {
        items_.push_back(item);
    }
}

void Menu::clearItems() {
    items_.clear();
    selectedIndex_ = 0;
    scrollOffset_ = 0;
}

void Menu::setItems(const std::vector<MenuItem>& items) {
    clearItems();
    addItems(items);
}

int Menu::run() {
    if (items_.empty()) {
        return -1;
    }
    
    // Find first selectable item
    while (selectedIndex_ < static_cast<int>(items_.size()) && 
           !isSelectable(items_[selectedIndex_])) {
        selectedIndex_++;
    }
    
    running_ = true;
    
    while (running_) {
        display();
        if (!handleInput()) {
            break;
        }
    }
    
    return selectedIndex_;
}

void Menu::display() {
    if (!gScreen) return;
    
    gScreen->clear();
    gScreen->updateDimensions();
    
    // Draw title bar
    gScreen->drawTitleBar(title_);
    
    // Draw subtitle if present
    if (!subtitle_.empty()) {
        gScreen->drawCenteredText(2, subtitle_, ColorScheme::NORMAL);
    }
    
    // Draw menu items
    drawItems();
    
    // Draw status bar
    if (!statusMessage_.empty()) {
        gScreen->drawStatusBar(statusMessage_, statusIsError_);
    }
    
    // Draw help bar
    gScreen->drawHelpBar(helpText_);
    
    gScreen->refresh();
}

bool Menu::handleInput() {
    int ch = gScreen->getInput();
    
    switch (ch) {
        case KEY_UP:
        case 'k':
            moveUp();
            break;
            
        case KEY_DOWN:
        case 'j':
            moveDown();
            break;
            
        case KEY_ENTER:
        case '\n':
        case '\r':
            handleEnter();
            break;
            
        case 'q':
        case 'Q':
            running_ = false;
            return false;
            
        case 27: // ESC
            // Check for ESC sequence (arrow keys)
            nodelay(stdscr, TRUE);
            int next = getch();
            nodelay(stdscr, FALSE);
            
            if (next == ERR) {
                // Pure ESC, go back
                running_ = false;
                selectedIndex_ = -1;
                return false;
            }
            break;
    }
    
    return true;
}

void Menu::setSelectedIndex(int index) {
    if (index >= 0 && index < static_cast<int>(items_.size())) {
        selectedIndex_ = index;
        ensureVisible();
    }
}

void Menu::setStatus(const std::string& status, bool isError) {
    statusMessage_ = status;
    statusIsError_ = isError;
}

void Menu::setHelp(const std::string& help) {
    helpText_ = help;
}

int Menu::getVisibleHeight() const {
    if (!gScreen) return 10;
    int height = gScreen->getHeight();
    // Title(1) + subtitle(1) + padding(2) + status(1) + help(1) + borders(2)
    return height - 8;
}

void Menu::ensureVisible() {
    int visibleHeight = getVisibleHeight();
    
    if (selectedIndex_ < scrollOffset_) {
        scrollOffset_ = selectedIndex_;
    } else if (selectedIndex_ >= scrollOffset_ + visibleHeight) {
        scrollOffset_ = selectedIndex_ - visibleHeight + 1;
    }
}

void Menu::moveDown() {
    if (items_.empty()) return;
    
    int start = selectedIndex_;
    do {
        selectedIndex_++;
        if (selectedIndex_ >= static_cast<int>(items_.size())) {
            selectedIndex_ = 0;
        }
    } while (!isSelectable(items_[selectedIndex_]) && selectedIndex_ != start);
    
    ensureVisible();
}

void Menu::moveUp() {
    if (items_.empty()) return;
    
    int start = selectedIndex_;
    do {
        selectedIndex_--;
        if (selectedIndex_ < 0) {
            selectedIndex_ = static_cast<int>(items_.size()) - 1;
        }
    } while (!isSelectable(items_[selectedIndex_]) && selectedIndex_ != start);
    
    ensureVisible();
}

void Menu::handleEnter() {
    if (selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(items_.size())) {
        return;
    }
    
    MenuItem& item = items_[selectedIndex_];
    
    if (!item.enabled) return;
    
    switch (item.type) {
        case MenuItemType::Action:
        case MenuItemType::Submenu:
            if (item.action) {
                item.action();
            }
            break;
            
        case MenuItemType::Back:
            running_ = false;
            selectedIndex_ = -1;
            break;
            
        case MenuItemType::Toggle:
            if (item.action) {
                item.action();
            }
            break;
            
        case MenuItemType::Input:
            if (item.action) {
                item.action();
            }
            break;
            
        default:
            break;
    }
}

void Menu::drawItems() {
    if (!gScreen) return;
    
    int width = gScreen->getWidth();
    int visibleHeight = getVisibleHeight();
    int startY = subtitle_.empty() ? 3 : 4;
    int startX = 4;
    int itemWidth = width - 8;
    
    // Draw box around menu
    gScreen->drawBox(startY - 1, startX - 2, visibleHeight + 2, itemWidth + 4, ColorScheme::BORDER);
    
    // Draw items
    for (int i = 0; i < visibleHeight && (scrollOffset_ + i) < static_cast<int>(items_.size()); i++) {
        int itemIndex = scrollOffset_ + i;
        const MenuItem& item = items_[itemIndex];
        int y = startY + i;
        
        bool isSelected = (itemIndex == selectedIndex_);
        
        // Draw separator
        if (item.type == MenuItemType::Separator) {
            gScreen->drawHLine(y, startX, itemWidth, ColorScheme::BORDER);
            continue;
        }
        
        // Determine color
        int colorPair = ColorScheme::NORMAL;
        if (isSelected) {
            colorPair = ColorScheme::HIGHLIGHT;
        } else if (!item.enabled) {
            colorPair = ColorScheme::BORDER;
        }
        
        // Clear line
        attron(COLOR_PAIR(colorPair));
        for (int x = 0; x < itemWidth; x++) {
            mvaddch(y, startX + x, ' ');
        }
        
        // Draw item
        std::string prefix = "  ";
        if (item.type == MenuItemType::Back) {
            prefix = "< ";
        } else if (item.type == MenuItemType::Submenu) {
            prefix = "> ";
        } else if (item.type == MenuItemType::Toggle) {
            prefix = item.value == "on" ? "[*] " : "[ ] ";
        }
        
        std::string displayText = prefix + item.label;
        
        // Truncate if needed
        if (static_cast<int>(displayText.length()) > itemWidth - 2) {
            displayText = displayText.substr(0, itemWidth - 5) + "...";
        }
        
        mvprintw(y, startX + 1, "%s", displayText.c_str());
        
        // Draw description on right side for selected item
        if (isSelected && !item.description.empty()) {
            int descX = itemWidth - static_cast<int>(item.description.length()) - 2;
            if (descX > static_cast<int>(displayText.length()) + 3) {
                mvprintw(y, startX + descX, "%s", item.description.c_str());
            }
        }
        
        attroff(COLOR_PAIR(colorPair));
    }
    
    // Draw scroll indicators
    if (scrollOffset_ > 0) {
        gScreen->drawText(startY - 1, width / 2, "▲", ColorScheme::TITLE);
    }
    if (scrollOffset_ + visibleHeight < static_cast<int>(items_.size())) {
        gScreen->drawText(startY + visibleHeight, width / 2, "▼", ColorScheme::TITLE);
    }
}

bool Menu::isSelectable(const MenuItem& item) const {
    return item.type != MenuItemType::Separator && item.enabled;
}

} // namespace tui
} // namespace easytty
