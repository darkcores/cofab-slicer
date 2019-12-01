#ifndef SLICEWIDGET_H
#define SLICEWIDGET_H

#include <QWidget>

class SliceWidget : public QWidget {
    Q_OBJECT
  public:
    SliceWidget(QWidget *parent = nullptr);

    void setSlice(const std::vector<QPolygon> *slice);
    void setColor(const QColor color);
    void setRandomColor(const bool random);

  protected:
    void paintEvent(QPaintEvent *event);

  private:
	const std::vector<QPolygon> *lines;
	QColor color;
    bool random_color = true;
};

#endif // SLICEWIDGET_H
