#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "user-interface/views/mapviewwindow.h"

class MainWindow : public UserInterface::MapViewWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void processBrushes();
    void importTextures();

public slots:
    void init();

protected:
    virtual void initShaders() override;
    virtual void initTextures() override;
    virtual void initMaterials() override;
    virtual void initLocalOpenGlSettings() override;

private:
    quint32 m_iPlaceholderMaterial;
};

#endif // MAINWINDOW_H
