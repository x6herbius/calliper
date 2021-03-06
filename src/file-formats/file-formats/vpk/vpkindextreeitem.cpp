#include "vpkindextreeitem.h"

namespace FileFormats
{
    namespace
    {
        template<typename T>
        inline QString toHex(const T& val, bool leading0x = false)
        {
            return QString(leading0x ? "0x%1" : "%1").arg(val, sizeof(T)*2, 16);
        }
    }

    struct VPKIndexTreeItem::Data
    {
        quint32 crc;
        quint16 preloadBytes;
        quint16 archiveIndex;
        quint32 entryOffset;
        quint32 entryLength;
        quint16 terminator;     // Should always be 0xffff.
    };

    VPKIndexTreeItem::VPKIndexTreeItem()
        : StreamDataContainer(),
          m_pData(new VPKIndexTreeItem::Data())
    {
        clear();
    }

    VPKIndexTreeItem::VPKIndexTreeItem(const VPKIndexTreeItem &other)
        : StreamDataContainer(),
          m_pData(new VPKIndexTreeItem::Data())
    {
        *this = other;
    }

    VPKIndexTreeItem& VPKIndexTreeItem::operator =(const VPKIndexTreeItem& other)
    {
        memcpy(m_pData, other.m_pData, sizeof(VPKIndexTreeItem::Data));
        m_PreloadData = other.m_PreloadData;

        return *this;
    }

    VPKIndexTreeItem::~VPKIndexTreeItem()
    {
        delete m_pData;
    }

    quint32 VPKIndexTreeItem::staticSize()
    {
        return (3 * sizeof(quint32)) + (3 * sizeof(quint16));
    }

    quint32 VPKIndexTreeItem::totalSize() const
    {
        return staticSize() + m_PreloadData.length();
    }

    quint32 VPKIndexTreeItem::fileSize() const
    {
        return m_pData->preloadBytes + m_pData->entryLength;
    }

    bool VPKIndexTreeItem::populate(QDataStream &stream, QString *errorHint)
    {
        VPKIndexTreeItem::Data& d = *m_pData;

        beginRead(stream);

        stream >> d.crc
                >> d.preloadBytes
                >> d.archiveIndex
                >> d.entryOffset
                >> d.entryLength
                >> d.terminator;

        if ( d.preloadBytes > 0 )
        {
            m_PreloadData.resize(d.preloadBytes);
            stream.readRawData(m_PreloadData.data(), d.preloadBytes);
        }
        else
        {
            m_PreloadData.clear();
        }

        if ( !endRead(stream, errorHint) )
            return false;

        // Double check this just in case.
        if ( d.terminator != 0xffff )
        {
            setErrorString(errorHint, QString("Unexpected terminator value %1").arg(toHex<quint16>(d.terminator, true)));
            return false;
        }

        return true;
    }

    void VPKIndexTreeItem::clear()
    {
        memset(m_pData, 0, sizeof(VPKIndexTreeItem::Data));
    }

    quint32 VPKIndexTreeItem::crc() const
    {
        return m_pData->crc;
    }

    quint16 VPKIndexTreeItem::preloadBytes() const
    {
        return m_pData->preloadBytes;
    }

    quint16 VPKIndexTreeItem::archiveIndex() const
    {
        return m_pData->archiveIndex;
    }

    quint32 VPKIndexTreeItem::entryOffset() const
    {
        return m_pData->entryOffset;
    }

    quint32 VPKIndexTreeItem::entryLength() const
    {
        return m_pData->entryLength;
    }

    QByteArray VPKIndexTreeItem::preloadData() const
    {
        return m_PreloadData;
    }

    QString VPKIndexTreeItem::containerName() const
    {
        return "VPK Index Tree Item";
    }
}
