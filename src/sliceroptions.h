#ifndef SLICEROPTIONS_H
#define SLICEROPTIONS_H

#include <QDockWidget>

class QCheckBox;
class QLineEdit;

class SlicerOptions : public QDockWidget {
    Q_OBJECT
  public:
    explicit SlicerOptions(QWidget *parent = nullptr);

  signals:

  public slots:
};

#endif // SLICEROPTIONS_H
