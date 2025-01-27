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
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

  private:
    QPoint lastpoint;
    int zoomlvl = 0;
    std::vector<QPolygon> lines, original;
    QColor color;
    bool random_color = true;
    bool mouse_down = false;

  public slots:
    void zoomIn();
    void zoomOut();
};

#endif // SLICEWIDGET_H
