#ifndef ORBITCAMERACONTROLLER_H
#define ORBITCAMERACONTROLLER_H

#include <Qt3DExtras/QAbstractCameraController>

class OrbitCameraController : public Qt3DExtras::QAbstractCameraController {
    Q_OBJECT

  public:
    OrbitCameraController(Qt3DCore::QNode *parent = nullptr);

  private:
    virtual void moveCamera(const InputState &state, float dt);
};

#endif // ORBITCAMERACONTROLLER_H
