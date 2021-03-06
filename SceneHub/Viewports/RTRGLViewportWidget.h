/*
 *  Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
 */

#ifndef __UI_SCENEHUB_RTR_GLVIEWPORT_WIDGET_H__
#define __UI_SCENEHUB_RTR_GLVIEWPORT_WIDGET_H__

#include <QDrag>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGLWidget>
#include <FabricUI/SceneHub/SHGLScene.h>
#include <FabricUI/SceneHub/SHGLRenderer.h>
#include <FabricUI/Viewports/ViewportWidget.h>

namespace FabricUI {
namespace Viewports {
  
class RTRGLViewportWidget : public ViewportWidget {

  Q_OBJECT

  public:
    RTRGLViewportWidget(
      FabricCore::Client &client,
      SceneHub::SHGLRenderer *shGLRenderer,
      SceneHub::SHGLScene *shGLScene,
      int,
      QGLContext*,
      QWidget *parent = 0,
      QGLWidget *shared = 0,
      QSettings *settings = 0 );

    virtual ~RTRGLViewportWidget();
    
    void toggleAlwaysRefresh();
    
    bool alwaysRefreshes() { return m_alwaysRefresh; }
    
    void setOrthographic( bool orthographic );
    
    bool isOrthographic() { return m_orthographic; }

    virtual FabricCore::RTVal getCamera();
    
    virtual void setBackgroundColor( QColor color ) {};

    int getViewportIndex() { return m_viewportIndex; }

    /// Special case: when changing the 'sampling' settings, the widget needs to be recreated
    void detachFromRTRViewport();


  public slots:
    void onSceneUpdated(FabricUI::SceneHub::SHGLScene *shGLScene) { m_shGLScene = shGLScene; } 


  signals:
    void sceneChanged();
    
    void viewportDestroying();
    
    void manipsAcceptedEvent(bool);
    
    void synchronizeCommands();

    void redrawOnAlwaysRefresh(); 

  private:
    bool onEvent(QEvent *event);

    virtual void paintGL();
    
    virtual void resizeGL(int w, int h);

    virtual void enterEvent(QEvent * event);
    
    virtual void leaveEvent(QEvent * event);
    
    virtual void wheelEvent(QWheelEvent *event) { onEvent(event); }
    
    virtual void keyPressEvent(QKeyEvent *event) { onEvent(event); }
    
    virtual void keyReleaseEvent(QKeyEvent *event) { onEvent(event); }
    
    virtual void mouseMoveEvent(QMouseEvent *event) { onEvent(event); }
    
    virtual void mousePressEvent(QMouseEvent *event);
    
    virtual void mouseReleaseEvent(QMouseEvent *event);

    virtual void dropEvent(QDropEvent *event);
    
    virtual void dragMoveEvent(QDragMoveEvent *event);
    
    virtual void dragEnterEvent(QDragEnterEvent *event);


    QWidget *m_parent;
    QPoint m_screenPos;
    int m_viewportIndex;
    bool m_alwaysRefresh;
    bool m_orthographic;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_samples;
    SceneHub::SHGLRenderer *m_shGLRenderer;
    SceneHub::SHGLScene *m_shGLScene;
};

} // namespace Viewports
} // namespace FabricUI

#endif // __UI_SCENEHUB_RTR_GLVIEWPORT_WIDGET_H__
