#include "vpkinfo.h"
#include "file-formats/vpk/vpkheader.h"
#include <QtDebug>
#include <QTextStream>
#include "file-formats/vpk/vpkfile.h"
#include <QCryptographicHash>

namespace VPKInfo
{
    namespace
    {
        template<typename T>
        inline QString toHex(const T& val, bool leading0x = false)
        {
            return QString(leading0x ? "0x%1" : "%1").arg(val, sizeof(T)*2, 16);
        }
    }

    void printHeaderData(const FileFormats::VPKHeader &header, const QStringList& siblingArchives)
    {

        QString string;
        QTextStream s(&string);

        s << "======================================\n"
          << "=               Header               =\n"
          << "======================================\n";

#define HEADER_FIELD(_key, _value) \
        qSetFieldWidth(21) << QString("%1:").arg(_key).toUtf8().constData() <<\
        qSetFieldWidth(17) << (_value) << qSetFieldWidth(0)

        s << HEADER_FIELD("File signature", toHex<quint32>(header.signature(), true)) << endl;
        s << HEADER_FIELD("Version",        header.version()) << endl;
        s << HEADER_FIELD("Tree size",      QString("%1 bytes").arg(header.treeSize())) << endl;

        if ( header.version() != 2 )
            return;

        s << HEADER_FIELD("File data section",      QString("%1 bytes").arg(header.fileDataSectionSize())) << endl;
        s << HEADER_FIELD("Archive MD5 section",    QString("%1 bytes").arg(header.archiveMD5SectionSize())) << endl;
        s << HEADER_FIELD("Other MD5 section",      QString("%1 bytes").arg(header.otherMD5SectionSize())) << endl;
        s << HEADER_FIELD("Signature section",      QString("%1 bytes").arg(header.signatureSectionSize())) << endl;

#undef HEADER_FIELD

        s << endl;
        s << siblingArchives.count() << " sibling archives detected.";
        if ( siblingArchives.count() > 0 )
        {
            s << endl << siblingArchives.join("\n") << endl;
        }

        s.flush();
        qInfo() << qPrintable(string);
    }

    void printIndexData(const FileFormats::VPKIndex &index)
    {
        QString string;
        QTextStream s(&string);

        s << "======================================\n"
          << "=               Index                =\n"
          << "======================================\n";

        QStringList extensions = index.extensions();
        s << "Files: " << index.recordCount() << ", extensions: " << extensions.count() << endl << endl;

#define FIELD(_key, _value) \
        qSetFieldWidth(8) << QString("%1:").arg(_key).toUtf8().constData() <<\
        qSetFieldWidth(12) << (_value) << qSetFieldWidth(0)

        foreach ( const QString& str, extensions )
        {
            s << FIELD(str, QString("%1 files").arg(index.recordCountForExtension(str))) << endl;
        }

#undef FIELD

        s.flush();
        qInfo() << qPrintable(string);
    }

    void listFiles(const FileFormats::VPKIndex &index)
    {
        qInfo() << "======================================";
        qInfo() << "=               Files                =";
        qInfo() << "======================================\n";

        QStringList paths = index.recordPaths();
        foreach ( const QString& str, paths )
        {
            qInfo().noquote() << str;
        }

        qInfo() << "";
    }

    void printArchiveMD5Data(const FileFormats::VPKArchiveMD5Collection &collection, const QStringList& archives, bool verbose)
    {
        using namespace FileFormats;

        qInfo() << "======================================";
        qInfo() << "=       Archive MD5 Checksums        =";
        qInfo() << "======================================\n";

        if ( collection.count() < 1 )
        {
            qInfo() << "No checksums present.\n";
            return;
        }

        if ( archives.count() < 1 )
        {
            qCritical() << "No sibling archives present in directory, cannot verify checksums.\n";
            return;
        }

        QSet<quint32> indices = collection.archiveIndices();

        qInfo() << "VPK contains" << collection.count() << "MD5 checksums across"
                 << indices.count() << "sibling archives.";

        if ( indices.count() != archives.count() )
        {
            qWarning() << "Warning: checksums reference" << indices.count()
                       << "sibling archives but" << archives.count()
                       << "were detected in the directory.";
        }

        qInfo() << "Verifying checksums...";

        quint32 currentArchive = ~0;
        QFile archive;
        bool hasFail = false;
        for (int i = 0; i < collection.count(); i++)
        {
            VPKArchiveMD5ItemPointer item = collection.itemAt(i);

            if ( item->archiveIndex() >= static_cast<quint32>(archives.count()) )
            {
                qCritical() << "Archive index" << item->archiveIndex() << "is invalid.\n";
                return;
            }

            if ( item->archiveIndex() != currentArchive )
            {
                archive.close();

                currentArchive = item->archiveIndex();
                archive.setFileName(archives.at(currentArchive));
                if ( !archive.open(QIODevice::ReadOnly) )
                {
                    qCritical().nospace() << "Could not open archive " << currentArchive
                                << " (" << archive.fileName() << ").\n";
                    return;
                }

                qInfo().nospace() << "Verifying archive " << currentArchive << "...";
            }

            archive.seek(item->startingOffset());
            QByteArray inputData = archive.read(item->count());

            if ( static_cast<quint32>(inputData.length()) != item->count() )
            {
                qCritical() << "Expected to read" << item->count()
                            << "bytes from archive" << item->archiveIndex()
                            << "for verifying checksum, but could only read"
                            << inputData.length() << "bytes.\n";
                return;
            }

            QString line;
            if ( verbose )
            {
                line = QString("Computing checksum for archive %1, offset %2, count %3...")
                        .arg(currentArchive)
                        .arg(item->startingOffset())
                        .arg(item->count());
            }

            bool hashMatched = QCryptographicHash::hash(inputData, QCryptographicHash::Md5) == item->md5();

            if ( hashMatched )
            {
                if ( verbose )
                    line += " PASS.";
            }
            else
            {
                if ( verbose )
                    line += " FAIL.";

                hasFail = true;
            }

            if ( verbose )
                qInfo() << qPrintable(line);
        }

        archive.close();

        if ( verbose )
            qInfo() << "";

        if ( hasFail )
        {
            qInfo() << "One or more checksum verifications failed.\n";
        }
        else
        {
            qInfo() << "All checksum verifications passed.\n";
        }
    }

    void printOtherMD5Data(const FileFormats::VPKOtherMD5Item &md5s,
                           const QByteArray &treeData, const QByteArray &archiveMD5Data)
    {
        using namespace FileFormats;

        qInfo() << "======================================";
        qInfo() << "=         Other MD5 Checksums        =";
        qInfo() << "======================================\n";

        if ( treeData.length() < 1 )
        {
            qCritical() << "No tree data to validate against checksum.";
            return;
        }

        if ( archiveMD5Data.length() < 1 )
        {
            qCritical() << "No archive MD5 data to validate against checksum.";
            return;
        }

        QString str = "Validating tree data checksum...";

        if ( QCryptographicHash::hash(treeData, QCryptographicHash::Md5) == md5s.treeChecksum() )
        {
            str += " PASS.";
        }
        else
        {
            str += " FAIL.";
        }

        qInfo() << qPrintable(str);

        str = "Validating archive MD5 data checksum...";

        if ( QCryptographicHash::hash(archiveMD5Data, QCryptographicHash::Md5) == md5s.archiveMD5SectionChecksum() )
        {
            str += " PASS.";
        }
        else
        {
            str += " FAIL.";
        }

        qInfo() << qPrintable(str) << "";
    }

    void printInfoAboutFile(const FileFormats::VPKIndex &index, const QString &filename)
    {
        using namespace FileFormats;

        VPKIndexTreeRecordPointer record = index.recordAt(filename);

        if ( record.isNull() )
        {
            qInfo() << "File" << filename << "not found in VPK.";
            return;
        }

        QString string;
        QTextStream s(&string);
        s << "File: " << record->fullPath() << endl;

        VPKIndexTreeItem* item = record->item();

#define PRINT_PAIR(_key, _value) \
    left << qSetFieldWidth(15) << (_key) << qSetFieldWidth(0) << (_value)

        s << PRINT_PAIR("Path:", record->path()) << endl;
        s << PRINT_PAIR("Name:", record->fileName()) << endl;
        s << PRINT_PAIR("Extension:", record->extension()) << endl;
        s << PRINT_PAIR("Archive index:", item->archiveIndex()) << endl;
        s << PRINT_PAIR("Offset:", item->entryOffset()) << endl;
        s << PRINT_PAIR("Length:", item->entryLength()) << endl;

#undef PRINT_PAIR

        s.flush();
        qInfo() << qPrintable(string);
    }

    void outputFile(FileFormats::VPKFile &file, const QString &filePath, const QString &outputPath)
    {
        using namespace FileFormats;

        VPKIndexTreeRecordPointer record = file.index().recordAt(filePath);
        if ( record.isNull() )
        {
            qWarning() << "Could not find file" << filePath << "in VPK.";
            return;
        }

        if ( !file.openArchive(record->item()->archiveIndex()) )
        {
            qWarning() << "Could not open VPK archive" << record->item()->archiveIndex()
                       << "to load file" << filePath;
            return;
        }

        QByteArray data = file.readFromCurrentArchive(record->item());
        file.closeArchive();

        QFile outFile(outputPath);
        if ( !outFile.open(QIODevice::WriteOnly) )
        {
            qWarning() << "Unable to open file" << outputPath << "for writing.";
            return;
        }

        outFile.write(data);
        outFile.close();

        qInfo() << "File" << filePath << "extracted from VPK successfully as" << outputPath;
    }
}
