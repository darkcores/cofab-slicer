#include "src/mainwindow.h"

#include <QApplication>
#include <QUrl>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
	if (argc == 2) {
		w.loadFile(QUrl(argv[1]));
	}
    w.show();
    return a.exec();
}
