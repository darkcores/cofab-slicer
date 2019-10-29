#ifndef STLVIEWER_H
#define STLVIEWER_H

#include <QDockWidget>
#include <QUrl>

namespace Qt3DCore {
class QEntity;
}

namespace Qt3DExtras {
	class QDiffuseSpecularMaterial;
}

class STLViewer : public QDockWidget {
    Q_OBJECT
  public:
    explicit STLViewer(QWidget *parent = nullptr);

	void loadStl(QUrl file);

  private:
	Qt3DCore::QEntity *rootEntity, *stlEntity;
	Qt3DExtras::QDiffuseSpecularMaterial *material;
	void drawLine(const QVector3D& start, const QVector3D& end, const QColor& color);
	
    Qt3DCore::QEntity *createScene();

  signals:

  public slots:
};

#endif // STLVIEWER_H
