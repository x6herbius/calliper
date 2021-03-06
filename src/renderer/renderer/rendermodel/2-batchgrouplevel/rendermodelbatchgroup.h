#ifndef RENDERMODELBATCHGROUP_H
#define RENDERMODELBATCHGROUP_H

#include "renderer_global.h"
#include <QSet>
#include "renderer/rendermodel/3-batchlevel/openglbatch.h"
#include <QHash>
#include "renderer/rendermodel/3-batchlevel/matrixbatchkey.h"
#include "renderer/rendermodel/3-batchlevel/matrixbatch.h"
#include <QOpenGLBuffer>
#include "renderer/shaders/vertexformat.h"
#include <QPair>
#include "renderer/shaders/ishaderspec.h"
#include <QOpenGLShaderProgram>
#include "renderer/rendermodel/2-batchgrouplevel/rendermodelbatchgroupkey.h"

namespace Renderer
{
    class RenderModelBatchGroup
    {
    public:
        typedef QSharedPointer<MatrixBatch> MatrixBatchPointer;

        RenderModelBatchGroup(const RenderModelBatchGroupKey &key, QOpenGLBuffer::UsagePattern usagePattern, const IShaderSpec* shaderSpec);
        ~RenderModelBatchGroup();

        QOpenGLBuffer::UsagePattern usagePattern() const;
        const RenderModelBatchGroupKey& key() const;

        MatrixBatchPointer createMatrixBatch(const MatrixBatchKey &key);
        MatrixBatchPointer getMatrixBatch(const MatrixBatchKey &key) const;
        void removeMatrixBatch(const MatrixBatchKey &key);
        bool containsMatrixBatch(const MatrixBatchKey &key) const;
        int matrixBatchCount() const;
        void clear();
        void setMatrixBatchDrawable(const MatrixBatchKey &key, bool drawable);
        bool matrixBatchDrawable(const MatrixBatchKey &key) const;

        void printDebugInfo() const;

        void drawAllBatches(QOpenGLShaderProgram* shaderProgram);

    private:
        typedef QSharedPointer<OpenGLBatch> OpenGLBatchPointer;

        void setFull(const OpenGLBatchPointer &batch);
        void setWaiting(const OpenGLBatchPointer &batch);
        OpenGLBatchPointer getNextWaitingGlBatch();
        void draw(QSet<OpenGLBatchPointer> &batches, QOpenGLShaderProgram* shaderProgram);
        void ensureAllBatchesUploaded(QSet<OpenGLBatchPointer> &batches);
        void addMatrixBatchToOpenGLBatch(const MatrixBatchKey &key);
        void removeMatrixBatchFromOpenGLBatch(const MatrixBatchKey &key);

        QSet<OpenGLBatchPointer>  m_WaitingBatches; // Batches that have space for more matrices.
        QSet<OpenGLBatchPointer>  m_FullBatches;    // Batches that have no more space.

        QHash<MatrixBatchKey, MatrixBatchPointer> m_MatrixBatches;
        QHash<MatrixBatchKey, OpenGLBatchPointer> m_MatrixOpenGLMap;

        const RenderModelBatchGroupKey m_Key;   // For convenience
        const QOpenGLBuffer::UsagePattern m_iUsagePattern;
        const IShaderSpec* m_pShaderSpec;

        GLenum  m_iDrawMode;
        float   m_flDrawWidth;
    };
}

#endif // RENDERMODELBATCHGROUP_H
