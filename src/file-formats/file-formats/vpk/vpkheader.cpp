#include "vpkheader.h"
#include <QtDebug>

namespace FileFormats
{
    namespace
    {
        const quint32 VPK_SIGNATURE = 0x55aa1234;
    }

    struct VPKHeader::Data
    {
        quint32 signature;
        quint32 version;

        quint32 treeSize;
        quint32 fileDataSectionSize;
        quint32 archiveMD5SectionSize;
        quint32 otherMD5SectionSize;
        quint32 signatureSectionSize;
    };

    VPKHeader::VPKHeader()
        : StreamDataContainer(),
          m_pData(new VPKHeader::Data())
    {
        clear();
    }

    VPKHeader::~VPKHeader()
    {
        delete m_pData;
    }

    quint32 VPKHeader::size() const
    {
        if ( m_pData->version == 2 )
            return 7 * sizeof(quint32);

        return 2 * sizeof(quint32);
    }

    bool VPKHeader::populate(QDataStream &stream, QString *errorHint)
    {
        VPKHeader::Data& d = *m_pData;

        beginRead(stream);

        stream >> d.signature >> d.version;

        if ( d.version == 2 )
        {
            stream
                    >> d.treeSize
                    >> d.fileDataSectionSize
                    >> d.archiveMD5SectionSize
                    >> d.otherMD5SectionSize
                    >> d.signatureSectionSize;
        }

        if ( !endRead(stream, errorHint) )
            return false;

        if ( !signatureValid() )
        {
            setErrorString(errorHint, QString("Invalid VPK signature 0x%1.").arg(d.signature, 4, 16));
            return false;
        }

        if ( d.version < 1 || d.version > 2 )
        {
            setErrorString(errorHint, QString("Unexpected VPK version '%1'.").arg(d.version));
            return false;
        }

        return true;
    }

    void VPKHeader::clear()
    {
        memset(m_pData, 0, sizeof(VPKHeader::Data));
    }

    quint32 VPKHeader::signature() const
    {
        return m_pData->signature;
    }

    quint32 VPKHeader::version() const
    {
        return m_pData->version;
    }

    quint32 VPKHeader::treeSize() const
    {
        return m_pData->treeSize;
    }

    quint32 VPKHeader::fileDataSectionSize() const
    {
        return m_pData->fileDataSectionSize;
    }

    quint32 VPKHeader::archiveMD5SectionSize() const
    {
        return m_pData->archiveMD5SectionSize;
    }

    quint32 VPKHeader::otherMD5SectionSize() const
    {
        return m_pData->otherMD5SectionSize;
    }

    quint32 VPKHeader::signatureSectionSize() const
    {
        return m_pData->signatureSectionSize;
    }

    bool VPKHeader::signatureValid() const
    {
        return m_pData->signature == VPK_SIGNATURE;
    }

    QString VPKHeader::containerName() const
    {
        return "VPK Header";
    }

    quint32 VPKHeader::treeAbsOffset() const
    {
        return size();
    }

    quint32 VPKHeader::fileDataSectionAbsOffset() const
    {
        return treeAbsOffset()
                + m_pData->treeSize;
    }

    quint32 VPKHeader::archiveMD5SectionAbsOffset() const
    {
        return fileDataSectionAbsOffset()
                + m_pData->fileDataSectionSize;
    }

    quint32 VPKHeader::otherMD5SectionAbsOffset() const
    {
        return archiveMD5SectionAbsOffset()
                + m_pData->archiveMD5SectionSize;
    }

    quint32 VPKHeader::signatureSectionAbsOffset() const
    {
        return otherMD5SectionAbsOffset()
                + m_pData->otherMD5SectionSize;
    }
}
