#ifndef FILESTORAGEMANAGER_H
#define FILESTORAGEMANAGER_H

#include <QString>

class FileStorageManager
{
public:
    static QString getStorageRoot();
    static QString getClientFolder(int clientId);
    static QString getProjectFolder(int projectId);

    static bool ensureFolderExists(const QString &path);
    static QString copyFileToStorage(const QString &sourceFilePath, const QString &destinationFolder);
    static QString getTemplateFolder(int templateId);
};

#endif // FILESTORAGEMANAGER_H
