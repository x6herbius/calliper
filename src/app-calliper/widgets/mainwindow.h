#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "app-calliper_global.h"
#include <QMainWindow>
#include "model/applicationproject.h"

class QTreeWidget;

namespace Ui {
class MainWindow;
}

namespace UserInterface
{
    class QuadGridWidget;
}

namespace AppCalliper
{
    class VisibleActionDockWidget;
    class ProjectFileDockWidget;
    class ProjectMetadataDockWidget;

    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private slots:
        void menuOpenProject();
        void menuSaveCurrentProject();
        void menuSaveCurrentProjectAs();
        void menuCloseProject();
        void menuNewProject();
        void notifyProjectDataChanged();

        void fileDoubleClicked(const QString& localPath);
        void addNewProjectFiles();

    private:
        void initDockWidgets();
        void initDockWidget(VisibleActionDockWidget* widget, QAction* action, Qt::DockWidgetArea area);
        void saveCurrentProject(const QString& fullPath);
        QString getFileDialogueDefaultPath() const;
        void updateWindowTitle();
        void repopulateProjectFileTree();
        void setProject(ApplicationProject* newProject);
        bool ensureProjectIsSaved();
        void setFileMenuItemEnabledStates();
        UserInterface::QuadGridWidget* centralGridWidget() const;
        QString getFullPath(const QString& localFilePath) const;

        Ui::MainWindow *ui;

        ProjectFileDockWidget* m_pProjectFileDockWidget;
        ProjectMetadataDockWidget* m_pProjectMetadataDockWidget;

        QScopedPointer<ApplicationProject> m_pProject;
        bool m_bUnsavedProjectChanges;
    };
}

#endif // MAINWINDOW_H
