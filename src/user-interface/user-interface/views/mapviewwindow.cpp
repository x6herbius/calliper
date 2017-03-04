#include "mapviewwindow.h"
#include "model/shaders/unlitshader.h"
#include "model/shaders/unlitpervertexcolorshader.h"
#include "renderer/opengl/openglerrors.h"
#include "renderer/opengl/openglhelpers.h"
#include <QMouseEvent>
#include <QFile>
#include "file-formats/keyvalues/keyvaluesparser.h"
#include <QtDebug>
#include "model/shaders/errorshader.h"
#include "model/genericbrush/genericbrush.h"
#include "model/shaders/simplelitshader.h"
#include <QtGlobal>
#include <QMap>
#include "file-formats/vpk/vpkindextreerecord.h"
#include "model-loaders/vtf/vtfloader.h"
#include "model-loaders/filedataloaders/vmf/vmfdataloader.h"

using namespace Model;
using namespace Renderer;

namespace UserInterface
{
    MapViewWindow::MapViewWindow(IRenderPassClassifier *classifier) :
        QOpenGLWindow(),
        m_strMapPath(),
        m_strVpkPath(),
        m_bInitialised(false),
        m_pVmfData(nullptr),
        m_pRenderer(nullptr),
        m_pRenderPassClassifier(classifier),
        m_pSceneRenderer(nullptr),
        m_ShaderPalette(),
        m_pCameraController(nullptr),
        m_pKeyMap(nullptr),
        m_pMouseEventMap(nullptr)
    {
    }

    MapViewWindow::~MapViewWindow()
    {
        makeCurrent();
        destroy();
        doneCurrent();
    }

    void MapViewWindow::destroy()
    {
        delete m_pMouseEventMap;
        m_pMouseEventMap = nullptr;

        delete m_pKeyMap;
        m_pKeyMap = nullptr;

        delete m_pCameraController;
        m_pCameraController = nullptr;

        delete m_pSceneRenderer;
        m_pSceneRenderer = nullptr;

        delete m_pRenderer;
        m_pRenderer = nullptr;

        delete m_pVmfData;
        m_pVmfData = nullptr;

        ResourceEnvironment::globalShutdown();

        delete m_pRenderPassClassifier;
        m_pRenderPassClassifier = nullptr;
    }

    void MapViewWindow::initializeGL()
    {
        initLocalOpenGlSettings();

        ResourceEnvironment::globalInitialise();

        initShaders();
        initTextures();
        initMaterials();

        m_pVmfData = new MapFileDataModel();

        m_pRenderer = new Renderer::RenderModel();
        initRenderer();

        m_pSceneRenderer = new SceneRenderer(m_pRenderPassClassifier,
                                             m_pRenderer,
                                             m_pVmfData->scene());
        initSceneRenderer();

        m_pCameraController = new CameraController(this);
        initCameraController();

        m_pKeyMap = new KeyMap(this);
        initKeyMap();

        m_pMouseEventMap = new MouseEventMap(this);
        initMouseEventMap();

        m_bInitialised = true;
    }

    void MapViewWindow::paintGL()
    {
        if ( !m_bInitialised )
        {
            return;
        }

        m_pSceneRenderer->setShaderPalette(m_ShaderPalette);

        GL_CURRENT_F;
        GLTRY(f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        m_pSceneRenderer->render(m_pVmfData->scene()->defaultCamera());
    }

    void MapViewWindow::resizeGL(int w, int h)
    {
        SceneCamera* camera = m_pVmfData->scene()->defaultCamera();

        CameraLens lens = camera->lens();
        lens.setAspectRatio(static_cast<float>(w)/static_cast<float>(h));
        camera->setLens(lens);
    }

    void MapViewWindow::initShaders()
    {
    }

    void MapViewWindow::initTextures()
    {
    }

    void MapViewWindow::initMaterials()
    {
    }

    void MapViewWindow::initLocalOpenGlSettings()
    {
    }

    void MapViewWindow::initRenderer()
    {
        m_pRenderer->setShaderFunctor(ResourceEnvironment::globalInstance()->shaderStore());
        m_pRenderer->setTextureFunctor(ResourceEnvironment::globalInstance()->textureStore());
        m_pRenderer->setMaterialFunctor(ResourceEnvironment::globalInstance()->materialStore());
    }

    void MapViewWindow::initScene()
    {

    }

    void MapViewWindow::initSceneRenderer()
    {

    }

    void MapViewWindow::initCameraController()
    {
        m_pCameraController->setCamera(m_pVmfData->scene()->defaultCamera());
        m_pCameraController->setStrafeSpeed(100.0f);
        m_pCameraController->setForwardSpeed(100.0f);
        m_pCameraController->setVerticalSpeed(100.0f);
        m_pCameraController->setEnabled(true);

        connect(m_pCameraController, &CameraController::tickFinished, this, [this]{ update(); });
    }

    void MapViewWindow::initKeyMap()
    {
        connect(m_pKeyMap->addKeyMap(Qt::Key_W), &KeySignalSender::keyEvent,
                m_pCameraController, &CameraController::moveForward);
        connect(m_pKeyMap->addKeyMap(Qt::Key_S), &KeySignalSender::keyEvent,
                m_pCameraController, &CameraController::moveBackward);
        connect(m_pKeyMap->addKeyMap(Qt::Key_A), &KeySignalSender::keyEvent,
                m_pCameraController, &CameraController::moveLeft);
        connect(m_pKeyMap->addKeyMap(Qt::Key_D), &KeySignalSender::keyEvent,
                m_pCameraController, &CameraController::moveRight);
        connect(m_pKeyMap->addKeyMap(Qt::Key_Q), &KeySignalSender::keyEvent,
                m_pCameraController, &CameraController::moveUp);
        connect(m_pKeyMap->addKeyMap(Qt::Key_Z), &KeySignalSender::keyEvent,
                m_pCameraController, &CameraController::moveDown);
        connect(m_pKeyMap->addKeyMap(Qt::Key_Escape), &KeySignalSender::keyEvent,
                this, &MapViewWindow::close);

        installEventFilter(m_pKeyMap);
    }

    void MapViewWindow::initMouseEventMap()
    {
        m_pMouseEventMap->setHorizontalSensitivity(-1);
        connect(m_pMouseEventMap, &MouseEventMap::mouseMovedX, m_pCameraController, &CameraController::addYaw);
        connect(m_pMouseEventMap, &MouseEventMap::mouseMovedY, m_pCameraController, &CameraController::addPitch);
        connect(m_pMouseEventMap, &MouseEventMap::mouseMovedX, this, [this]{ update(); });
        connect(m_pMouseEventMap, &MouseEventMap::mouseMovedY, this, [this]{ update(); });

        installEventFilter(m_pMouseEventMap);
    }

    void MapViewWindow::mousePressEvent(QMouseEvent *e)
    {
        if ( e->button() == Qt::LeftButton )
        {
            m_pMouseEventMap->setEnabled(true);
        }
        else
        {
            QOpenGLWindow::mousePressEvent(e);
        }
    }

    void MapViewWindow::mouseReleaseEvent(QMouseEvent *e)
    {
        if ( e->button() == Qt::LeftButton )
        {
            m_pMouseEventMap->setEnabled(false);
        }
        else
        {
            QOpenGLWindow::mousePressEvent(e);
        }
    }

    QString MapViewWindow::vpkPath() const
    {
        return m_strVpkPath;
    }

    void MapViewWindow::setVpkPath(const QString& path)
    {
        if ( m_strVpkPath == path )
            return;

        m_strVpkPath = path;
    }

    QString MapViewWindow::mapPath() const
    {
        return m_strMapPath;
    }

    void MapViewWindow::setMapPath(const QString& path)
    {
        if ( m_strMapPath == path )
            return;

        m_strMapPath = path;
    }

    Model::ShaderPalette& MapViewWindow::shaderPalette()
    {
        return m_ShaderPalette;
    }

    const Model::ShaderPalette& MapViewWindow::shaderPalette() const
    {
        return m_ShaderPalette;
    }

    Model::IRenderPassClassifier* MapViewWindow::renderPassClassifier()
    {
        return m_pRenderPassClassifier;
    }

    const Model::IRenderPassClassifier* MapViewWindow::renderPassClassifier() const
    {
        return m_pRenderPassClassifier;
    }

    const FileFormats::VPKFileCollection& MapViewWindow::vpkFileCollection() const
    {
        return m_VpkFiles;
    }

    Model::MapFileDataModel* MapViewWindow::mapDataModel()
    {
        return m_pVmfData;
    }

    const Model::MapFileDataModel* MapViewWindow::mapDataModel() const
    {
        return m_pVmfData;
    }

    void MapViewWindow::loadMap()
    {
        using namespace FileFormats;
        using namespace ModelLoaders;

        if ( m_strMapPath.isNull() || m_strMapPath.isEmpty() )
        {
            QMessageBox::critical(nullptr, "Error", "No VMF file provided.");
            return;
        }

        VmfDataLoader loader;
        loader.setDataModel(m_pVmfData);
        QString errorString;

        if ( loader.load(m_strMapPath, &errorString) != BaseFileLoader::Success )
        {
            QMessageBox msg(QMessageBox::Critical, "Error", "Unable to load VMF.", QMessageBox::Ok);
            if ( !errorString.isEmpty() )
            {
                msg.setDetailedText(errorString);
            }
        }
    }

    void MapViewWindow::loadVpks()
    {
        m_VpkFiles.clear();
        m_VpkFiles.addFilesFromDirectory(m_strVpkPath);
    }
}
