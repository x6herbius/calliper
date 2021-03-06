#ifndef OPENGLBATCH_H
#define OPENGLBATCH_H

#include "renderer_global.h"
#include <QOpenGLBuffer>
#include "renderer/opengl/opengluniformbuffer.h"
#include "renderer/shaders/vertexformat.h"
#include "matrixbatch.h"
#include "renderer/shaders/ishaderspec.h"
#include <QOpenGLShaderProgram>
#include "renderer/shaders/shaderdefs.h"
#include "matrixbatch.h"
#include "renderer/rendermodel/3-batchlevel/matrixbatchkey.h"

namespace Renderer
{
    class OpenGLBatch
    {
    public:
        typedef QSharedPointer<MatrixBatch> MatrixBatchPointer;

        OpenGLBatch(QOpenGLBuffer::UsagePattern usagePattern, const IShaderSpec* shaderSpec);
        ~OpenGLBatch();

        void create();
        void destroy();
        bool isCreated() const;
        void upload();

        const IShaderSpec* shaderSpec() const;
        VertexFormat vertexFormat() const;
        int batchSize() const;
        bool isSingleItemBatch() const;

        QOpenGLBuffer::UsagePattern usagePattern() const;

        void insertMatrixBatch(const MatrixBatchKey &key, const MatrixBatchPointer& batch);
        void removeMatrixBatch(const MatrixBatchKey &key);
        int matrixBatchCount() const;
        bool matrixBatchLimitReached() const;
        MatrixBatchPointer matrixBatchAt(const MatrixBatchKey &key) const;
        void clearMatrixBatches();

        // The state of this flag must be set externally if the contents
        // of a referenced matrix batch are changed.
        bool needsUpload() const;
        void setNeedsUpload(bool needsUpload);
        void uploadIfRequired();

        void setVertexAttributes(QOpenGLShaderProgram* shaderProgram) const;
        void bindAll();
        void draw();
        void releaseAll();

        void exportVertexData(QVector<float> &out);
        void exportIndexData(QVector<quint32> &out);
        void exportUniformData(QVector<float> &out);

        GLenum drawMode() const;
        void setDrawMode(GLenum mode);

    private:
        static inline quint32 maskFromNumberOfBits(int numBits)
        {
            return (quint32)(~0) >> ((sizeof(quint32) * 8) - numBits);
        }

        static inline quint8 bitsRequired(int maxValue)
        {
            for ( quint32 i = 0; i < sizeof(quint32)*8; i++ )
            {
                if ( 1 << i >= maxValue )
                {
                    return i+1;
                }
            }

            return sizeof(quint32)*8;
        }

        void uploadVertices();
        void uploadIndices();
        void uploadUniforms();
        void calculateRequiredSizeOfBuffers();
        void setAttributeBuffer(QOpenGLShaderProgram* shaderProgram,
                                Renderer::ShaderDefs::VertexArrayAttribute att,
                                int &offsetInBytes) const;
        void copyAllVertexData(char* buffer, int size);
        void updateObjectIds(char* buffer, int offset, int numBytes, quint32 id);

        const QOpenGLBuffer::UsagePattern m_iUsagePattern;
        bool m_bCreated;
        const IShaderSpec* m_pShaderSpec;
        const quint32 m_iMaskBitsRequired;
        const quint32 m_iObjectIdMask;

        QOpenGLBuffer       m_VertexBuffer;
        QOpenGLBuffer       m_IndexBuffer;
        OpenGLUniformBuffer m_UniformBuffer;
        bool                m_bNeedsUpload;
        QHash<MatrixBatchKey, MatrixBatchPointer>   m_MatrixBatchTable;
        MatrixBatchItemMetadata m_UploadMetadata;

        GLenum m_iDrawMode;
    };
}

#endif // OPENGLBATCH_H
