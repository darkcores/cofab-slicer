#include "stlviewer.h"
#include "orbitcameracontroller.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QPushButton>

#include <Qt3DCore/QAspectEngine>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QMesh>

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DInput/QInputAspect>

#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QGoochMaterial>
#include <Qt3DExtras/QFirstPersonCameraController>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DRender/QRenderAspect>

STLViewer::STLViewer(QWidget *parent) : QDockWidget(parent) {
    auto view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));

    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    view->registerAspect(input);

    Qt3DCore::QEntity *scene = createScene();

    // Camera
    Qt3DRender::QCamera *camera = view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f,
                                             1000.0f);
    camera->setPosition(QVector3D(40.0f, 40.0f, 40.0f));
    camera->setUpVector(QVector3D(0, 0, 1));
    camera->setViewCenter(QVector3D(0, 0, 0));

    // For camera controls
    auto camController = new OrbitCameraController(scene);
    camController->setLinearSpeed(70.0f);
    camController->setLookSpeed(240.0f);
    camController->setCamera(camera);

    view->setRootEntity(scene);
    view->show();

    auto w = createWindowContainer(view, this);

	// Setup layout with items
	auto layout = new QVBoxLayout();
	layout->addWidget(w);
	layout->addWidget(setupControlWidget());
	
	auto base = new QWidget(this);
	base->setLayout(layout);
    setWidget(base);
}

Qt3DCore::QEntity *STLViewer::createScene() {
    rootEntity = new Qt3DCore::QEntity;

    material = new Qt3DExtras::QGoochMaterial(rootEntity);
    material->setDiffuse(QColor(125, 54, 254));
	material->setCool(QColor(130, 130, 130));
	material->setWarm(QColor(50, 50, 50));

    // Torus
    stlEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DExtras::QTorusMesh *torusMesh = new Qt3DExtras::QTorusMesh;
    torusMesh->setRadius(5);
    torusMesh->setMinorRadius(1);
    torusMesh->setRings(100);
    torusMesh->setSlices(20);

    Qt3DCore::QTransform *torusTransform = new Qt3DCore::QTransform;
    torusTransform->setScale3D(QVector3D(1.5, 1, 0.5));
    torusTransform->setRotation(
        QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 45.0f));

    stlEntity->addComponent(torusMesh);
    stlEntity->addComponent(torusTransform);
    stlEntity->addComponent(material);

    // Setup axes
    drawLine({0, 0, 0}, {10, 0, 0}, Qt::red);   // X
    drawLine({0, 0, 0}, {0, 10, 0}, Qt::green); // Y
    drawLine({0, 0, 0}, {0, 0, 10}, Qt::blue);  // Z

    return rootEntity;
}

void STLViewer::loadStl(QUrl file) {
    // remove old entity
    delete stlEntity;

    stlEntity = new Qt3DCore::QEntity(rootEntity);

    Qt3DRender::QMesh *stlMesh = new Qt3DRender::QMesh;
    stlMesh->setMeshName("StlFileMesh");
    stlMesh->setSource(file);
    stlEntity->addComponent(stlMesh);
    stlEntity->addComponent(material);
}

void STLViewer::drawLine(const QVector3D &start, const QVector3D &end,
                         const QColor &color) {
    auto *geometry = new Qt3DRender::QGeometry(rootEntity);

    // position vertices (start and end)
    QByteArray bufferBytes;
    bufferBytes.resize(
        3 * 2 *
        sizeof(float)); // start.x, start.y, start.end + end.x, end.y, end.z
    float *positions = reinterpret_cast<float *>(bufferBytes.data());
    *positions++ = start.x();
    *positions++ = start.y();
    *positions++ = start.z();
    *positions++ = end.x();
    *positions++ = end.y();
    *positions++ = end.z();

    auto *buf = new Qt3DRender::QBuffer(geometry);
    buf->setData(bufferBytes);

    auto *positionAttribute = new Qt3DRender::QAttribute(geometry);
    positionAttribute->setName(
        Qt3DRender::QAttribute::defaultPositionAttributeName());
    positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
    positionAttribute->setVertexSize(3);
    positionAttribute->setAttributeType(
        Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(buf);
    positionAttribute->setByteStride(3 * sizeof(float));
    positionAttribute->setCount(2);
    geometry->addAttribute(
        positionAttribute); // We add the vertices in the geometry

    // connectivity between vertices
    QByteArray indexBytes;
    indexBytes.resize(2 * sizeof(unsigned int)); // start to end
    unsigned int *indices = reinterpret_cast<unsigned int *>(indexBytes.data());
    *indices++ = 0;
    *indices++ = 1;

    auto *indexBuffer = new Qt3DRender::QBuffer(geometry);
    indexBuffer->setData(indexBytes);

    auto *indexAttribute = new Qt3DRender::QAttribute(geometry);
    indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setBuffer(indexBuffer);
    indexAttribute->setCount(2);
    geometry->addAttribute(indexAttribute); // We add the indices linking the
                                            // points in the geometry

    // mesh
    auto *line = new Qt3DRender::QGeometryRenderer(rootEntity);
    line->setGeometry(geometry);
    line->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    auto *material = new Qt3DExtras::QGoochMaterial(rootEntity);
    material->setDiffuse(color);
	material->setWarm(color);
	material->setCool(color);
	
    // entity
    auto *lineEntity = new Qt3DCore::QEntity(rootEntity);
    lineEntity->addComponent(line);
    lineEntity->addComponent(material);
}

QWidget *STLViewer::setupControlWidget() {
	auto layout = new QHBoxLayout();
	auto colorBtn = new QPushButton("Color", this);
	connect(colorBtn, &QPushButton::clicked, this, &STLViewer::changeColor);
	layout->addWidget(colorBtn);
	layout->addWidget(new QPushButton("Reset view"));
	layout->addWidget(new QPushButton("Top"));
	layout->addWidget(new QPushButton("Front"));
	layout->addWidget(new QPushButton("Side"));

	auto w = new QWidget(this);
	w->setLayout(layout);
	return w;
}

void STLViewer::changeColor() {
	auto newColor = QColorDialog::getColor(material->diffuse(), this);
	material->setDiffuse(newColor);
}
