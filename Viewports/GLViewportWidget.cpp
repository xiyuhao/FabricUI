/*
 *  Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
 */

#include "GLViewportWidget.h"
 

using namespace FabricUI::Viewports;

GLViewportWidget::GLViewportWidget(
  FabricCore::Client &client, 
  QColor bgColor, 
  QGLFormat format, 
  QWidget *parent, 
  QSettings *settings)
: ViewportWidget(client, bgColor, format, parent, settings)
{	
  m_manipTool = new ManipulationTool(this);
  setAutoBufferSwap(false);

  m_gridVisible = true; // default value
  if(m_settings)
  {
    if(m_settings->contains("glviewport/gridVisible"))
      m_gridVisible = m_settings->value("glviewport/gridVisible").toBool();
  }

  try
  {
    m_client.loadExtension("Manipulation", "", false);
    m_client.loadExtension("InlineDrawing", "", false);
  }
  catch(FabricCore::Exception e)
  {
    printf("Error: %s\n", e.getDesc_cstr());
  }
  
  m_resizedOnce  = false;
  resetRTVals( false /*shouldUpdateGL*/ );
}

GLViewportWidget::~GLViewportWidget()
{
  delete(m_manipTool);
}

void GLViewportWidget::setBackgroundColor(QColor color)
{
  m_bgColor = color;

  if(m_viewport.isValid())
  {
    try
    {
      FabricCore::RTVal bgColorVal = m_viewport.maybeGetMember("bgColor");
      bgColorVal.setMember("r", FabricCore::RTVal::ConstructFloat32(m_client, float(m_bgColor.red()) / 255.0f));
      bgColorVal.setMember("g", FabricCore::RTVal::ConstructFloat32(m_client, float(m_bgColor.green()) / 255.0f));
      bgColorVal.setMember("b", FabricCore::RTVal::ConstructFloat32(m_client, float(m_bgColor.blue()) / 255.0f));
      m_viewport.setMember("bgColor", bgColorVal);
    }
    catch(FabricCore::Exception e)
    {
      printf("Error: %s\n", e.getDesc_cstr());
    }
  }
}

bool GLViewportWidget::isManipulationActive() 
{
  return m_manipTool->isActive();
}

void GLViewportWidget::setManipulationActive(bool state)
{
  if(state == m_manipTool->isActive())
    return;
  if(state)
    m_manipTool->toolOnSetup();
  else
    m_manipTool->toolOffCleanup();
  redraw();
}

void GLViewportWidget::clearInlineDrawing()
{
  if(m_viewport.isValid() && m_drawContext.isValid())
  {
    resetRTVals();
    initializeGL();
    resizeGL(size().width(), size().height());
  }
}

void GLViewportWidget::initializeGL()
{
  try
  {
    m_viewport.callMethod("", "setup", 1, &m_drawContext);
    m_drawing = m_drawing.callMethod("OGLInlineDrawing", "getInstance", 0, 0);
    setGridVisible(m_gridVisible, false);
    emit initComplete();
  }
  catch(FabricCore::Exception e)
  {
    printf("Error: %s\n", e.getDesc_cstr());
  }
}

void GLViewportWidget::resizeGL(int width, int height)
{
  m_width = width;
  m_height = height;

  try
  {
    FabricCore::RTVal args[3];
    args[0] = m_drawContext;
    args[1] = FabricCore::RTVal::ConstructUInt32(m_client, (unsigned int)m_width);
    args[2] = FabricCore::RTVal::ConstructUInt32(m_client, (unsigned int)m_height);
    m_viewport.callMethod("", "resize", 3, args);
    m_resizedOnce = true;
  }
  catch(FabricCore::Exception e)
  {
    printf("Error: %s\n", e.getDesc_cstr());
  }
}

void GLViewportWidget::paintGL()
{
  if(!m_resizedOnce)
  {
    QSize scale = size();
    resizeGL(scale.width(), scale.height());
  }

  ViewportWidget::computeFPS();

  try
  {
    FabricCore::RTVal args[2];
    args[0] = FabricCore::RTVal::ConstructString(m_client, "default");
    args[1] = m_drawContext;
    m_drawing.callMethod("", "drawViewport", 2, args);
  }
  catch(FabricCore::Exception e)
  {
    printf("Error: %s\n", e.getDesc_cstr());
  }

  swapBuffers();
  emit redrawn();
}

void GLViewportWidget::resetRTVals( bool shouldUpdateGL )
{
  try
  {
    m_drawing = FabricCore::RTVal::Create(m_client, "OGLInlineDrawing", 0, 0);
    if(!m_drawing.isValid())
    {
      printf("[GLWidget] Error: Cannot construct OGLInlineDrawing RTVal (extension loaded?)\n");
      return;
    }
    m_drawing = m_drawing.callMethod("OGLInlineDrawing", "getNewInstance", 0, 0);

    m_viewport = FabricCore::RTVal::Create(m_client, "OGLStandaloneViewport", 0, 0);
    if(!m_viewport.isValid())
    {
      printf("[GLWidget] Error: Cannot construct OGLStandaloneViewport RTVal (extension loaded?)\n");
      return;
    }
    else
    {
      FabricCore::RTVal args[2];
      args[0] = FabricCore::RTVal::ConstructString(m_client, "default");
      args[1] = m_viewport;
      m_drawing.callMethod("", "registerViewport", 2, args);

      // [pzion 20150909] No viewport overlay, at least for now
      // if(!m_hasCommercialLicense)
      //   m_viewport.callMethod("", "setupLicensingOverlay", 0, 0);
    }

    m_camera = m_viewport.maybeGetMember("camera");
    m_cameraManipulator = FabricCore::RTVal::Create(m_client, "CameraManipulator", 1, &m_camera);

    m_viewport.setMember("windowId", FabricCore::RTVal::ConstructUInt64(m_client, (uint64_t)this->winId()));

    setBackgroundColor(m_bgColor);

    m_drawContext = FabricCore::RTVal::Create(m_client, "DrawContext", 0, 0);
    if(!m_drawContext.isValid())
    {
      printf("[GLWidget] Error: Cannot construct DrawContext RTVal (extension loaded?)\n");
      return;
    }
    m_drawContext = m_drawContext.callMethod("DrawContext", "getInstance", 0, 0);
    m_drawContext.callMethod("", "resetAllViewportParams", 0, 0);

    // [pzion 20150728] Artificially push a meaningless movement
    // event to force camera manipulation code to compiler, preventing
    // annoying pauses
    QMouseEvent nullEvent(
      QMouseEvent::MouseMove,
      QCursor::pos(),
      Qt::NoButton,
      Qt::NoButton,
      Qt::NoModifier
      );
    manipulateCamera(
      &nullEvent,
      false, // requireModifier
      false // shouldUpdateGL
      );
  }
  catch(FabricCore::Exception e)
  {
    printf("Error: %s\n", e.getDesc_cstr());
  }

  setGridVisible( m_gridVisible, shouldUpdateGL );
}

void GLViewportWidget::mousePressEvent(QMouseEvent *event)
{
  if(manipulateCamera(event))
    return;
  if(m_manipTool->onEvent(event))
    return;
  ViewportWidget::mousePressEvent(event);
  FabricCore::FlagUserInteraction();
}

void GLViewportWidget::mouseMoveEvent(QMouseEvent *event)
{
  if(manipulateCamera(event))
    return;
  if(m_manipTool->onEvent(event))
    return;
  ViewportWidget::mouseMoveEvent(event);
}

void GLViewportWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if(manipulateCamera(event))
    return;
  if(m_manipTool->onEvent(event))
    return;
  ViewportWidget::mouseReleaseEvent(event);
}

void GLViewportWidget::wheelEvent(QWheelEvent *event)
{
  if(manipulateCamera(event, m_manipTool->isActive()))
    return;
  if(m_manipTool->onEvent(event))
    return;
  ViewportWidget::wheelEvent(event);
}

bool GLViewportWidget::manipulateCamera(
  QInputEvent *event,
  bool requireModifier,
  bool shouldUpdateGL
  )
{
  if(!event->modifiers().testFlag(Qt::AltModifier) && requireModifier)
    return false;

  bool result = false;
  try
  {
    // Now we translate the Qt events to FabricEngine events..
    FabricCore::RTVal klevent = QtToKLEvent(event, m_client, m_viewport, "Canvas");

    // And then pass the event to the camera manipulator for handling.
    m_cameraManipulator.callMethod("", "onEvent", 1, &klevent);
    result = klevent.callMethod("Boolean", "isAccepted", 0, 0).getBoolean();
  }
  catch(FabricCore::Exception e)
  {
    printf("Error: %s\n", e.getDesc_cstr());
  }
  if ( shouldUpdateGL )
    update();
  return result;
}

bool GLViewportWidget::isGridVisible()
{
  return m_gridVisible;
}

void GLViewportWidget::setGridVisible( bool gridVisible, bool updateView )
{
  m_gridVisible = gridVisible;
  if(m_settings)
  {
    m_settings->setValue("glviewport/gridVisible", m_gridVisible);
  }

  if(!m_viewport.isValid())
    return;

  try
  {
    FabricCore::RTVal gridVisibleVal = FabricCore::RTVal::ConstructBoolean(m_client, m_gridVisible);
    m_viewport.callMethod("", "setGridVisible", 1, &gridVisibleVal);
  }
  catch(FabricCore::Exception e)
  {
    printf("Error: %s\n", e.getDesc_cstr());
  }

  if( updateView )
    update();
}

void GLViewportWidget::resetCamera()
{
  if(!m_viewport.isValid())
    return;

  try
  {
    FabricCore::RTVal position[3];
    position[0] = FabricCore::RTVal::ConstructFloat32(m_client, 30.0f);
    position[1] = FabricCore::RTVal::ConstructFloat32(m_client, 20.0f);
    position[2] = FabricCore::RTVal::ConstructFloat32(m_client, 40.0f);
    
    FabricCore::RTVal target[3];
    target[0] = FabricCore::RTVal::ConstructFloat32(m_client, 0.0f);
    target[1] = FabricCore::RTVal::ConstructFloat32(m_client, 0.0f);
    target[2] = FabricCore::RTVal::ConstructFloat32(m_client, 0.0f);
    
    FabricCore::RTVal args[2];
    args[0] = FabricCore::RTVal::Construct(m_client, "Vec3", 3, position);
    args[1] = FabricCore::RTVal::Construct(m_client, "Vec3", 3, target);

    m_camera.callMethod("", "setFromPositionAndTarget", 2, args);
    m_cameraManipulator = FabricCore::RTVal::Create(m_client, "CameraManipulator", 1, &m_camera);
  }
  catch(FabricCore::Exception e)
  {
    printf("Error: %s\n", e.getDesc_cstr());
  }

  update();
}

