#pragma once

#include "common/Types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace easytty {
namespace tui {

/**
 * @brief Menu item structure
 */
struct MenuItem {
    std::string label;
    std::string description;
    MenuItemType type;
    std::function<void()> action;
    bool enabled;
    std::string value;  // For toggles/inputs
    
    MenuItem(const std::string& lbl, 
             const std::string& desc = "",
             MenuItemType t = MenuItemType::Action,
             std::function<void()> act = nullptr,
             bool en = true)
        : label(lbl), description(desc), type(t), action(act), enabled(en) {}
    
    static MenuItem Separator() {
        return MenuItem("", "", MenuItemType::Separator, nullptr, false);
    }
    
    static MenuItem Back(const std::string& label = "< Back") {
        return MenuItem(label, "Return to previous menu", MenuItemType::Back, nullptr, true);
    }
};

/**
 * @brief KConfig-style menu class
 */
class Menu {
public:
    Menu(const std::string& title, const std::string& subtitle = "");
    virtual ~Menu() = default;
    
    /**
     * @brief Add a menu item
     */
    void addItem(const MenuItem& item);
    
    /**
     * @brief Add multiple items
     */
    void addItems(const std::vector<MenuItem>& items);
    
    /**
     * @brief Clear all items
     */
    void clearItems();
    
    /**
     * @brief Set items
     */
    void setItems(const std::vector<MenuItem>& items);
    
    /**
     * @brief Run the menu
     * @return Index of selected item, -1 if escaped
     */
    int run();
    
    /**
     * @brief Display the menu once
     */
    void display();
    
    /**
     * @brief Handle input
     * @return True if menu should continue running
     */
    bool handleInput();
    
    /**
     * @brief Get selected index
     */
    int getSelectedIndex() const { return selectedIndex_; }
    
    /**
     * @brief Set selected index
     */
    void setSelectedIndex(int index);
    
    /**
     * @brief Get item count
     */
    size_t getItemCount() const { return items_.size(); }
    
    /**
     * @brief Set status message
     */
    void setStatus(const std::string& status, bool isError = false);
    
    /**
     * @brief Set help text
     */
    void setHelp(const std::string& help);

protected:
    std::string title_;
    std::string subtitle_;
    std::vector<MenuItem> items_;
    int selectedIndex_;
    int scrollOffset_;
    std::string statusMessage_;
    bool statusIsError_;
    std::string helpText_;
    bool running_;
    
    /**
     * @brief Get visible height for menu items
     */
    int getVisibleHeight() const;
    
    /**
     * @brief Ensure selected item is visible
     */
    void ensureVisible();
    
    /**
     * @brief Move to next selectable item
     */
    void moveDown();
    
    /**
     * @brief Move to previous selectable item
     */
    void moveUp();
    
    /**
     * @brief Handle enter on selected item
     */
    void handleEnter();
    
    /**
     * @brief Draw menu items
     */
    virtual void drawItems();
    
    /**
     * @brief Check if item is selectable
     */
    bool isSelectable(const MenuItem& item) const;
};

} // namespace tui
} // namespace easytty
