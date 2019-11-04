#ifndef STLVIEWER_H
#define STLVIEWER_H

#include <QDockWidget>
#include <QUrl>
#include <Qt3DRender/QMesh>

namespace Qt3DCore {
class QEntity;
}

namespace Qt3DExtras {
class QGoochMaterial;
} // namespace Qt3DExtras

class STLViewer : public QDockWidget {
    Q_OBJECT
  public:
    explicit STLViewer(QWidget *parent = nullptr);

    void loadStl(QUrl file);

  private:
    Qt3DCore::QEntity *rootEntity, *stlEntity;
    Qt3DExtras::QGoochMaterial *material;
    Qt3DRender::QMesh *stlMesh;

    void drawLine(const QVector3D &start, const QVector3D &end,
                  const QColor &color);
    QWidget *setupControlWidget();

    Qt3DCore::QEntity *createScene();

  private slots:
    void changeColor();
	// This is just for debugging purposes
    void geometryChange(Qt3DRender::QMesh::Status status) {
        qDebug() << status;
        if (stlMesh->geometry()) {
            qDebug() << stlMesh->geometry()->attributes().at(0);
        }
    }
};

#endif // STLVIEWER_H
