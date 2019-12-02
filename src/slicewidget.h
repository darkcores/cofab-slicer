#ifndef SLICEWIDGET_H
#define SLICEWIDGET_H

#include <QPolygon>
#include <QWidget>
#include <vector>

class SliceWidget : public QWidget {
    Q_OBJECT
  public:
    explicit SliceWidget(QWidget *parent = nullptr);

    void setSlice(const std::vector<QPolygon> slice);
    void setColor(const QColor color);
    void setRandomColor(const bool random);
    const QColor getColor() const { return color; }

  protected:
    void paintEvent(QPaintEvent *event);

  private:
    std::vector<QPolygon> lines;
    QColor color;
    bool random_color = true;
};

#endif // SLICEWIDGET_H
