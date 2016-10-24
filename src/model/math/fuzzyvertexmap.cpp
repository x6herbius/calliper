#include "fuzzyvertexmap.h"
#include "math/math.h"

namespace NS_MODEL
{
    FuzzyVertexMap::FuzzyVertexMap()
    {
        m_iNextIndex = 0;
    }

    int FuzzyVertexMap::count() const
    {
        return m_Map.count();
    }

    void FuzzyVertexMap::clear()
    {
        m_Map.clear();
        m_iNextIndex = 0;
    }

    int FuzzyVertexMap::mapToIndex(const QVector3D &vec)
    {
        bool isNull = NS_CALLIPERUTIL::Math::fuzzyVectorIsNull(vec);

        for ( VertexMap::const_iterator it = m_Map.cbegin(); it != m_Map.cend(); ++it )
        {
            if ( (isNull && NS_CALLIPERUTIL::Math::fuzzyVectorIsNull(it.key())) || NS_CALLIPERUTIL::Math::fuzzyVectorEquals(vec, it.key()) )
                return it.value();
        }

        m_Map.insert(vec, m_iNextIndex);
        m_iNextIndex++;
        return m_iNextIndex - 1;
    }

    QList<QVector3D> FuzzyVertexMap::vertexList() const
    {
        QList<QVector3D> list;
        list.reserve(m_Map.count());
        for ( int i = 0; i < m_Map.count(); i++ )
        {
            list.append(QVector3D());
        }

        for ( VertexMap::const_iterator it = m_Map.cbegin(); it != m_Map.cend(); ++it )
        {
            list[it.value()] = it.key();
        }

        return list;
    }
}
