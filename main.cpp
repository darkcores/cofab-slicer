#include "src/mainwindow.h"

#include "src/gcodegenerator.h"
#include "src/model3d.h"

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

/*
    Model3D test("../../../Downloads/10mm_test_cube.stl");
    GCodeGenerator g ;
    g.generateGcode(test.getSlices());
    return 0;
    */
}
