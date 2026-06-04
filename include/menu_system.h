#pragma once

#include <QString>
#include <QVector>

namespace BrainLLM {

struct MenuItem {
    QString text;
    QString action_id;
    QString shortcut;
    bool separator;
};

class MenuSystem {
public:
    MenuSystem();
    ~MenuSystem() = default;
    
    QVector<MenuItem> get_file_menu() const;
    QVector<MenuItem> get_edit_menu() const;
    QVector<MenuItem> get_view_menu() const;
    QVector<MenuItem> get_brain_menu() const;
    QVector<MenuItem> get_settings_menu() const;
    QVector<MenuItem> get_help_menu() const;
    
private:
    QVector<MenuItem> file_menu_;
    QVector<MenuItem> edit_menu_;
    QVector<MenuItem> view_menu_;
    QVector<MenuItem> brain_menu_;
    QVector<MenuItem> settings_menu_;
    QVector<MenuItem> help_menu_;
    
    void initialize_menus();
};

} // namespace BrainLLM
