#include "orbitcameracontroller.h"

#include <Qt3DRender/QCamera>

OrbitCameraController::OrbitCameraController(Qt3DCore::QNode *parent)
    : QAbstractCameraController(parent) {}

inline float clampInputs(float input1, float input2) {
    float axisValue = input1 + input2;
    return (axisValue < -1) ? -1 : (axisValue > 1) ? 1 : axisValue;
}

void OrbitCameraController::moveCamera(const InputState &state, float dt) {
    auto cam = camera();

    if (cam == nullptr)
        return;

    const QVector3D upVector(0.0f, 0.0f, 1.0f);

    if (state.leftMouseButtonActive) {
        cam->panAboutViewCenter((-state.rxAxisValue * lookSpeed()) * dt,
                                upVector);
        cam->tiltAboutViewCenter((-state.ryAxisValue * lookSpeed()) * dt);
    } else if (state.rightMouseButtonActive) {
        cam->translate(QVector3D(0, 0, state.ryAxisValue),
                       cam->DontTranslateViewCenter);
    } else if (state.middleMouseButtonActive) {
        cam->translate(
            QVector3D(-clampInputs(state.rxAxisValue, state.txAxisValue) *
                          linearSpeed(),
                      -clampInputs(state.ryAxisValue, state.tyAxisValue) *
                          linearSpeed(),
                      0) *
            dt);
    }
}
