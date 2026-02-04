#include <QApplication>
#include "main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("BrainLLM");
    app.setApplicationVersion("1.0.0");
    
    BrainLLM::MainWindow window;
    window.show();
    
    return app.exec();
}
