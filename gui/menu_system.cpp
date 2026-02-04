#include "menu_system.h"

namespace BrainLLM {

MenuSystem::MenuSystem() {
    initialize_menus();
}

void MenuSystem::initialize_menus() {
    // File Menu
    file_menu_.append({"&New", "new", "Ctrl+N", false});
    file_menu_.append({"&Open", "open", "Ctrl+O", false});
    file_menu_.append({"&Save", "save", "Ctrl+S", false});
    file_menu_.append({"", "", "", true});
    file_menu_.append({"&Exit", "exit", "Ctrl+Q", false});
    
    // Edit Menu
    edit_menu_.append({"&Undo", "undo", "Ctrl+Z", false});
    edit_menu_.append({"&Redo", "redo", "Ctrl+Y", false});
    edit_menu_.append({"", "", "", true});
    edit_menu_.append({"&Cut", "cut", "Ctrl+X", false});
    edit_menu_.append({"&Copy", "copy", "Ctrl+C", false});
    edit_menu_.append({"&Paste", "paste", "Ctrl+V", false});
    
    // View Menu
    view_menu_.append({"&Zoom In", "zoom_in", "Ctrl++", false});
    view_menu_.append({"Zoom &Out", "zoom_out", "Ctrl+-", false});
    view_menu_.append({"&Reset Zoom", "reset_zoom", "Ctrl+0", false});
    view_menu_.append({"", "", "", true});
    view_menu_.append({"&Full Screen", "fullscreen", "F11", false});
    
    // Brain Menu
    brain_menu_.append({"&Initialize", "init", "", false});
    brain_menu_.append({"&Reset", "reset", "", false});
    brain_menu_.append({"&Process Input", "process", "F5", false});
    brain_menu_.append({"&Generate", "generate", "F6", false});
    brain_menu_.append({"", "", "", true});
    brain_menu_.append({"&Train", "train", "F7", false});
    brain_menu_.append({"&View Memory", "memory", "F8", false});
    
    // Settings Menu
    settings_menu_.append({"&Preferences", "prefs", "", false});
    settings_menu_.append({"&API Settings", "api_settings", "", false});
    settings_menu_.append({"&Brain Config", "brain_config", "", false});
    settings_menu_.append({"", "", "", true});
    settings_menu_.append({"&Plugins", "plugins", "", false});
    
    // Help Menu
    help_menu_.append({"&About", "about", "", false});
    help_menu_.append({"&Documentation", "docs", "F1", false});
    help_menu_.append({"&Changelog", "changelog", "", false});
}

QVector<MenuItem> MenuSystem::get_file_menu() const {
    return file_menu_;
}

QVector<MenuItem> MenuSystem::get_edit_menu() const {
    return edit_menu_;
}

QVector<MenuItem> MenuSystem::get_view_menu() const {
    return view_menu_;
}

QVector<MenuItem> MenuSystem::get_brain_menu() const {
    return brain_menu_;
}

QVector<MenuItem> MenuSystem::get_settings_menu() const {
    return settings_menu_;
}

QVector<MenuItem> MenuSystem::get_help_menu() const {
    return help_menu_;
}

} // namespace BrainLLM
