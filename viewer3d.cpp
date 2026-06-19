#include "viewer3d.h"
#include <Qt3DRender/QCamera>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DCore/QTransform>
#include <QtGui/QColor>
#include <QDebug>
#include <QtMath>
#include <QtWidgets/QWidget>

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>

static Qt3DRender::QGeometryRenderer* createTriangularPrismMesh(Qt3DCore::QNode *parent, float w, float h, float d, float ox, float oz) {
    Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer(parent);
    Qt3DCore::QGeometry *geometry = new Qt3DCore::QGeometry(renderer);
    Qt3DCore::QBuffer *vertexBuffer = new Qt3DCore::QBuffer(geometry);

    QByteArray bufferBytes;
    bufferBytes.resize(8 * 3 * 6 * sizeof(float));
    float *ptr = reinterpret_cast<float*>(bufferBytes.data());

    // ЗАЩИТА ОТ КРАША ВИДЕОКАРТЫ: Проверяем векторы на ноль (NaN)
    auto addTri = [&](QVector3D v1, QVector3D v2, QVector3D v3) {
        QVector3D cross = QVector3D::crossProduct(v2 - v1, v3 - v1);
        // Если вектор нулевой, ставим дефолтную нормаль вверх, иначе нормализуем
        QVector3D n = (cross.lengthSquared() > 1e-6f) ? cross.normalized() : QVector3D(0, 1, 0);

        *ptr++ = v1.x(); *ptr++ = v1.y(); *ptr++ = v1.z();
        *ptr++ = n.x(); *ptr++ = n.y(); *ptr++ = n.z();
        *ptr++ = v2.x(); *ptr++ = v2.y(); *ptr++ = v2.z();
        *ptr++ = n.x(); *ptr++ = n.y(); *ptr++ = n.z();
        *ptr++ = v3.x(); *ptr++ = v3.y(); *ptr++ = v3.z();
        *ptr++ = n.x(); *ptr++ = n.y(); *ptr++ = n.z();
    };

    QVector3D b0(-ox, 0, -oz), bw(w - ox, 0, -oz), bd(-ox, 0, d - oz);
    QVector3D t0(-ox, h, -oz), tw(w - ox, h, -oz), td(-ox, h, d - oz);

    addTri(b0, bw, bd);
    addTri(t0, td, tw);
    addTri(b0, td, t0);
    addTri(b0, bd, td);
    addTri(b0, t0, tw);
    addTri(b0, tw, bw);
    addTri(bd, tw, td);
    addTri(bd, bw, tw);

    vertexBuffer->setData(bufferBytes);

    Qt3DCore::QAttribute *posAttr = new Qt3DCore::QAttribute(geometry);
    posAttr->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
    posAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
    posAttr->setVertexSize(3);
    posAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    posAttr->setBuffer(vertexBuffer);
    posAttr->setByteStride(6 * sizeof(float));
    posAttr->setByteOffset(0);
    posAttr->setCount(24);

    Qt3DCore::QAttribute *normAttr = new Qt3DCore::QAttribute(geometry);
    normAttr->setName(Qt3DCore::QAttribute::defaultNormalAttributeName());
    normAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
    normAttr->setVertexSize(3);
    normAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    normAttr->setBuffer(vertexBuffer);
    normAttr->setByteStride(6 * sizeof(float));
    normAttr->setByteOffset(3 * sizeof(float));
    normAttr->setCount(24);

    geometry->addAttribute(posAttr);
    geometry->addAttribute(normAttr);
    renderer->setGeometry(geometry);

    renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    renderer->setVertexCount(24);

    return renderer;
}

Viewer3D::Viewer3D(const QString &jsonFilePath)
{
    m_view = new Qt3DExtras::Qt3DWindow();
    m_view->defaultFrameGraph()->setClearColor(QColor(135, 206, 235));
    m_view->setTitle("3D Визуализация проекта");

    m_rootEntity = new Qt3DCore::QEntity();

    Qt3DRender::QCamera *camera = m_view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 10000.0f);
    camera->setPosition(QVector3D(9.0f, 15.0f, 15.0f));
    camera->setViewCenter(QVector3D(9.0f, 0.0f, 4.5f));

    Qt3DExtras::QOrbitCameraController *camController = new Qt3DExtras::QOrbitCameraController(m_rootEntity);
    camController->setLinearSpeed(50.0f);
    camController->setLookSpeed(180.0f);
    camController->setCamera(camera);

    setupEnvironment();
    buildSceneFromJson(jsonFilePath);

    m_view->setRootEntity(m_rootEntity);
}

void Viewer3D::show()
{
    // Оборачиваем 3D-сцену в стандартный QWidget
    QWidget *container = QWidget::createWindowContainer(m_view);
    container->setMinimumSize(1024, 768);
    container->setWindowTitle("3D Визуализация проекта");

    // ВАЖНО: Даем жесткую команду очищать видеопамять при закрытии крестиком
    container->setAttribute(Qt::WA_DeleteOnClose);

    // При уничтожении контейнера удаляем и сам класс Viewer3D из оперативной памяти
    QObject::connect(container, &QWidget::destroyed, [this]() {
        delete this;
    });

    container->show();
}

void Viewer3D::setupEnvironment()
{
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight *light = new Qt3DRender::QDirectionalLight(lightEntity);
    light->setColor("white");
    light->setIntensity(0.8f);
    light->setWorldDirection(QVector3D(-1.0f, -1.0f, -1.0f));
    lightEntity->addComponent(light);

    Qt3DCore::QEntity *grassEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DExtras::QPlaneMesh *grassMesh = new Qt3DExtras::QPlaneMesh();
    grassMesh->setWidth(100.0f);
    grassMesh->setHeight(100.0f);

    Qt3DExtras::QPhongMaterial *grassMaterial = new Qt3DExtras::QPhongMaterial();
    grassMaterial->setDiffuse(QColor(85, 170, 0));

    Qt3DCore::QTransform *grassTransform = new Qt3DCore::QTransform();
    grassTransform->setTranslation(QVector3D(9.0f, -0.01f, 4.5f));

    grassEntity->addComponent(grassMesh);
    grassEntity->addComponent(grassMaterial);
    grassEntity->addComponent(grassTransform);
}

void Viewer3D::createBox(float cx, float cz, float cy, float sizeX, float sizeY, float sizeZ, float angleY, QColor color)
{
    Qt3DCore::QEntity *boxEntity = new Qt3DCore::QEntity(m_rootEntity);

    Qt3DExtras::QCuboidMesh *boxMesh = new Qt3DExtras::QCuboidMesh();
    boxMesh->setXExtent(sizeX);
    boxMesh->setYExtent(sizeY);
    boxMesh->setZExtent(sizeZ);

    Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial();
    material->setDiffuse(color);

    Qt3DCore::QTransform *transform = new Qt3DCore::QTransform();
    transform->setTranslation(QVector3D(cx, cy + sizeY / 2.0f, cz));
    transform->setRotationY(angleY);

    boxEntity->addComponent(boxMesh);
    boxEntity->addComponent(material);
    boxEntity->addComponent(transform);
}

void Viewer3D::buildSceneFromJson(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonArray items = doc.object()["items"].toArray();
    float scale = 0.01f;

    QMap<QString, QJsonObject> wallsMap;
    for (const QJsonValue &val : items) {
        QJsonObject obj = val.toObject();
        if (obj["class_type"].toString() == "WallItem") {
            wallsMap[obj["name"].toString()] = obj;
        }
    }

    for (const QJsonValue &val : items) {
        QJsonObject obj = val.toObject();
        QString type = obj["class_type"].toString();

        float px = obj["pos_x"].toDouble() * scale;
        float pz = obj["pos_y"].toDouble() * scale;
        float rotation = obj["rotation"].toDouble();
        int level = obj["level_id"].toInt();
        float baseHeightY = (level - 1) * 3.0f;

        if (type == "WallItem") {
            float p1x = obj["p1_x"].toDouble() * scale;
            float p1y = obj["p1_y"].toDouble() * scale;
            float p2x = obj["p2_x"].toDouble() * scale;
            float p2y = obj["p2_y"].toDouble() * scale;

            float dx = p2x - p1x;
            float dz = p2y - p1y;
            float length = qSqrt(dx * dx + dz * dz);
            float thickness = obj["thickness"].toDouble() * scale;
            float height = obj["height"].toDouble();

            float nx = (length > 0) ? -dz / length : 0;
            float nz = (length > 0) ? dx / length : 0;

            int alignment = obj["alignment"].toInt();
            float offsetDist = (alignment == 0) ? (thickness / 2.0f) : ((alignment == 2) ? (-thickness / 2.0f) : 0.0f);

            float cx = px + (p1x + p2x) / 2.0f + (nx * offsetDist);
            float cz = pz + (p1y + p2y) / 2.0f + (nz * offsetDist);

            float wallAngle = qRadiansToDegrees(qAtan2(dz, dx));
            createBox(cx, cz, baseHeightY, length, height, thickness, -(rotation + wallAngle), QColor(230, 230, 225));
        }
        else if (type == "NodeItem") {
            // Размеры угла уже лежат в метрах!
            float nWidth = obj["width"].toDouble();
            float nDepth = obj["height"].toDouble();
            float nAngle = obj["rotation_angle"].toDouble();

            // Если высоты нет (старый файл), ставим стандартные 2.8м
            float nHeight = obj.contains("wall_height") && obj["wall_height"].toDouble() > 0.1f
                                ? obj["wall_height"].toDouble()
                                : 2.8f;

            // Компенсация оси вращения из 2D: setTransformOriginPoint(10.0, 10.0) -> 0.1 метра
            float ox = 10.0f * scale;
            float oz = 10.0f * scale;

            Qt3DCore::QEntity *nodeEntity = new Qt3DCore::QEntity(m_rootEntity);

            // Вызываем наш кастомный генератор Треугольника
            Qt3DRender::QGeometryRenderer *mesh = createTriangularPrismMesh(nodeEntity, nWidth, nHeight, nDepth, ox, oz);

            Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial();
            material->setDiffuse(QColor(230, 230, 225)); // Тот же цвет, что и у стен

            Qt3DCore::QTransform *transform = new Qt3DCore::QTransform();

            // В 3D ставим якорь ровно в ту же абсолютную точку, что и в 2D, и крутим
            transform->setTranslation(QVector3D(px + ox, baseHeightY, pz + oz));
            transform->setRotationY(-nAngle);

            nodeEntity->addComponent(mesh);
            nodeEntity->addComponent(material);
            nodeEntity->addComponent(transform);
        }
        else if (type == "WindowItem" || type == "DoorItem") {
            float width = obj["width"].toDouble() * scale;
            float height = obj["height"].toDouble();
            float elevation = (type == "WindowItem") ? obj["elevation"].toDouble() : 0.0f;

            QString hostWallName = obj["host_wall_name"].toString();
            float wallThickness = 0.20f;

            if (wallsMap.contains(hostWallName)) {
                wallThickness = wallsMap[hostWallName]["thickness"].toDouble() * scale;
            }

            float thickness = wallThickness + 0.04f;

            float cx = px;
            float cz = pz;

            QColor col = (type == "WindowItem") ? QColor(135, 206, 250, 200) : QColor(139, 69, 19);
            createBox(cx, cz, baseHeightY + elevation, width, height, thickness, -rotation, col);
        }
        else if (type == "FoundationBlockItem" || type == "ObjectItem") {
            float width = obj["width"].toDouble() * scale;
            float length = obj["length"].toDouble() * scale;
            float height = obj["height"].toDouble();

            float rad = qDegreesToRadians(rotation);
            float cx = px + (width / 2.0f) * qCos(rad) - (length / 2.0f) * qSin(rad);
            float cz = pz + (width / 2.0f) * qSin(rad) + (length / 2.0f) * qCos(rad);

            QColor col = (type == "FoundationBlockItem") ? QColor(80, 80, 80) : QColor(139, 69, 19);
            float startY = (type == "FoundationBlockItem") ? baseHeightY - height : baseHeightY;
            createBox(cx, cz, startY, width, height, length, -rotation, col);
        }
        else if (type == "FloorItem" || type == "RoofItem") {
            QJsonArray poly = obj["polygon"].toArray();
            if(poly.size() > 0) {
                float minX = 99999, maxX = -99999, minZ = 99999, maxZ = -99999;
                for (const QJsonValue &v : poly) {
                    float vx = v.toObject()["x"].toDouble() * scale;
                    float vz = v.toObject()["y"].toDouble() * scale;
                    if (vx < minX) minX = vx; if (vx > maxX) maxX = vx;
                    if (vz < minZ) minZ = vz; if (vz > maxZ) maxZ = vz;
                }

                float w = maxX - minX;
                float l = maxZ - minZ;
                float h = obj["height"].toDouble();

                float cx = px + (maxX + minX) / 2.0f;
                float cz = pz + (maxZ + minZ) / 2.0f;

                float startY = baseHeightY;
                if (type == "FloorItem") startY -= h;
                if (type == "RoofItem") startY += 2.8f;

                QColor col = (type == "FloorItem") ? QColor(150, 130, 100) : QColor(160, 50, 50);
                createBox(cx, cz, startY, w, h, l, -rotation, col);
            }
        }
    }
}
