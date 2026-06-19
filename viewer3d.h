#ifndef VIEWER3D_H
#define VIEWER3D_H

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DCore/QNode>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

class Viewer3D
{
public:
    Viewer3D(const QString &jsonFilePath);
    void show();

private:
    Qt3DExtras::Qt3DWindow *m_view;
    Qt3DCore::QEntity *m_rootEntity;

    void setupEnvironment();
    void buildSceneFromJson(const QString &filePath);

    void createBox(float cx, float cz, float cy, float sizeX, float sizeY, float sizeZ, float angleY, Qt3DExtras::QPhongMaterial *material);
};

#endif // VIEWER3D_H
