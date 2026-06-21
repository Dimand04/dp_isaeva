#include "filestoragemanager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QUuid>

QString FileStorageManager::getStorageRoot()
{
    QString path = QCoreApplication::applicationDirPath() + "/storage";
    ensureFolderExists(path);
    return path;
}

QString FileStorageManager::getClientFolder(int clientId)
{
    QString path = getStorageRoot() + QString("/clients/client_%1").arg(clientId);
    ensureFolderExists(path);
    return path;
}

QString FileStorageManager::getProjectFolder(int projectId)
{
    QString path = getStorageRoot() + QString("/projects/project_%1").arg(projectId);
    ensureFolderExists(path);
    return path;
}

bool FileStorageManager::ensureFolderExists(const QString &path)
{
    QDir dir;
    return dir.mkpath(path);
}

QString FileStorageManager::copyFileToStorage(const QString &sourceFilePath, const QString &destinationFolder)
{
    QFileInfo fileInfo(sourceFilePath);
    QString fileName = fileInfo.fileName();
    QString destPath = destinationFolder + "/" + fileName;

    if (QFile::exists(destPath)) {
        QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
        destPath = destinationFolder + "/" + fileInfo.baseName() + "_" + uuid + "." + fileInfo.completeSuffix();
    }

    if (QFile::copy(sourceFilePath, destPath)) {
        return destPath;
    }
    return QString();
}

QString FileStorageManager::getTemplateFolder(int templateId)
{
    QString path = getStorageRoot() + "/templates/template_" + QString::number(templateId);
    ensureFolderExists(path);
    return path;
}
