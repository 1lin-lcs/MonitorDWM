#include "monitorDWM.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    monitorDWM w;
    w.show();
    QStringList arguments = QCoreApplication::arguments();
    //qDebug() << arguments;
    if (arguments.size() > 1) {
        QString ar = arguments.at(1);
        if (ar == "min") {
            if (w.isVisible()) {
                w.showMinimized();
                w.hide();
            }
        }
    }
    return a.exec();
}
