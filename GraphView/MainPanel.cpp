// Copyright 2010-2015 Fabric Software Inc. All rights reserved.

#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QPainter>
#include <QtGui/QCursor>
#include <QtGui/QGraphicsView>

#include <FabricUI/GraphView/MainPanel.h>
#include <FabricUI/GraphView/Pin.h>
#include <FabricUI/GraphView/Node.h>
#include <FabricUI/GraphView/Graph.h>
#include <FabricUI/GraphView/GraphConfig.h>
#include <FabricUI/GraphView/CachingEffect.h>

#include <math.h>

using namespace FabricUI::GraphView;

const float MainPanel::s_minZoom = 0.05f;
const float MainPanel::s_maxZoom = 2.00f;
const float MainPanel::s_minZoomForOne = 1.0f / 1.01f;
const float MainPanel::s_maxZoomForOne = 1.01f;

MainPanel::MainPanel(Graph * parent)
: QGraphicsWidget(parent)
{
  m_graph = parent;
  m_mouseWheelZoomState = 1.0;
  m_mouseWheelZoomRate = m_graph->config().mouseWheelZoomRate;

  setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

  // m_backGround = new MainPanelBackground(this);
  m_itemGroup = new QGraphicsWidget(this);
  // m_itemGroup = new QGraphicsWidget(parent);

  m_manipulationMode = ManipulationMode_None;
  m_draggingSelRect = false;
  m_selectionRect = NULL;
  m_spaceBarDown = false;
}

Graph * MainPanel::graph()
{
  return m_graph;
}

const Graph * MainPanel::graph() const
{
  return m_graph;
}

QGraphicsWidget * MainPanel::itemGroup()
{
  return m_itemGroup;
}

const QGraphicsWidget * MainPanel::itemGroup() const
{
  return m_itemGroup;
}

float MainPanel::mouseWheelZoomRate() const
{
  return m_mouseWheelZoomRate;
}

void MainPanel::setMouseWheelZoomRate(float rate)
{
  m_mouseWheelZoomRate = rate;
}

float MainPanel::canvasZoom() const
{
  return m_mouseWheelZoomState;
}

void MainPanel::setCanvasZoom(float state, bool quiet)
{
  if(state > s_maxZoom)
    state = s_maxZoom;
  if(state < s_minZoom)
    state = s_minZoom;
  if(m_mouseWheelZoomState == state)
    return;

  m_mouseWheelZoomState = state;

  QGraphicsView * graphicsView = graph()->scene()->views()[0];
  if ( state > s_minZoomForOne
    && state < s_maxZoomForOne )
  {
    graphicsView->setRenderHint(QPainter::SmoothPixmapTransform, false);
    m_itemGroup->setScale(1.0f);
  }
  else
  {
    graphicsView->setRenderHint(QPainter::SmoothPixmapTransform, true);
    m_itemGroup->setScale(state);
  }

  update();

  if(!quiet)
    emit canvasZoomChanged(m_mouseWheelZoomState);
}

QPointF MainPanel::canvasPan() const
{
  return m_itemGroup->pos();
}

void MainPanel::setCanvasPan(QPointF pos, bool quiet)
{
  m_itemGroup->setPos(pos);

  if(!quiet)
    emit canvasPanChanged(pos);
}

MainPanel::ManipulationMode MainPanel::manipulationMode() const
{
  return m_manipulationMode;
}

QRectF MainPanel::boundingRect() const
{
  return m_boundingRect;
}

void MainPanel::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  // clean up the scene
  if(m_selectionRect)
  {
    scene()->removeItem(m_selectionRect);
    delete(m_selectionRect);
    m_selectionRect = NULL;
  }

  if(event->button() == Qt::LeftButton && !m_spaceBarDown && !event->modifiers().testFlag(Qt::AltModifier))
  {
    QPointF mouseDownPos = mapToItem(m_itemGroup, event->pos());
    m_selectionRect = new SelectionRect(this, mouseDownPos);
    m_draggingSelRect = false;

    if(!event->modifiers().testFlag(Qt::ControlModifier) && !event->modifiers().testFlag(Qt::ShiftModifier))
      m_graph->controller()->clearSelection();
    m_ongoingSelection.clear();

    m_manipulationMode = ManipulationMode_Select;
  }
  else if((event->button() == DFG_QT_MIDDLE_MOUSE && event->modifiers().testFlag(Qt::AltModifier))
    || (event->button() == Qt::LeftButton && m_spaceBarDown)
    || (event->button() == Qt::LeftButton && event->modifiers().testFlag(Qt::AltModifier))
    )
  {
    setCursor(Qt::OpenHandCursor);
    m_manipulationMode = ManipulationMode_Pan;
    m_lastPanPoint = event->pos();
  }
  else if(event->button() == Qt::RightButton && event->modifiers().testFlag(Qt::AltModifier))
  {
    setCursor(Qt::OpenHandCursor);
    m_manipulationMode = ManipulationMode_Zoom;
    m_lastPanPoint = event->pos();
    m_mouseAltZoomState = m_mouseWheelZoomState;
  }
  else if(event->button() == Qt::RightButton)
  {
    QMenu * menu = graph()->getGraphContextMenu();
    if(menu)
    {
      menu->exec(QCursor::pos());
      menu->deleteLater();
    }
    else
      QGraphicsWidget::mousePressEvent(event);
  }
  else
    QGraphicsWidget::mousePressEvent(event);

}

void MainPanel::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  if(m_manipulationMode == ManipulationMode_Select)
  {
    QPointF dragPoint = mapToItem(m_itemGroup, event->pos());
    m_selectionRect->setDragPoint(dragPoint);

    if(!event->modifiers().testFlag(Qt::ControlModifier) && !event->modifiers().testFlag(Qt::ShiftModifier))
      m_graph->controller()->clearSelection();
    else
    {
      for(size_t i=0;i<m_ongoingSelection.size();i++)
        m_graph->controller()->selectNode(m_ongoingSelection[i], false);
    }

    m_ongoingSelection.clear();
    std::vector<Node*> nodes = m_graph->nodes();
    for(size_t i=0;i<nodes.size();i++)
    {
      if(m_selectionRect->collidesWithItem(nodes[i]))
      {
        if(!nodes[i]->selected())
        {
          m_graph->controller()->selectNode(nodes[i], true);
          m_ongoingSelection.push_back(nodes[i]);
        }
      }
    }
    m_draggingSelRect = true;
  }
  else if(m_manipulationMode == ManipulationMode_Pan)
  {
    QTransform xfo = m_itemGroup->transform().inverted();
    QPointF delta = xfo.map(event->pos()) - xfo.map(m_lastPanPoint);
    m_lastPanPoint = event->pos();
    if(m_graph->controller()->panCanvas(delta + canvasPan()))
    {
      // if(!m_graph->config().mainPanelBackGroundPanFixed)
      // m_backGround->setOffset(m_itemGroup->transform());
      update();
    }
  }
  else if(m_manipulationMode == ManipulationMode_Zoom)
  {
    QPointF delta = event->pos() - m_lastPanPoint;
    float zoomFactor = powf( 1.005f, delta.x() - delta.y() );
    performZoom( m_mouseAltZoomState * zoomFactor, m_lastPanPoint );
  }
  else
    QGraphicsWidget::mouseMoveEvent(event);
}

void MainPanel::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  if(m_manipulationMode == ManipulationMode_Select)
  {
    prepareGeometryChange();
  
    scene()->removeItem(m_selectionRect);
    delete(m_selectionRect);
    m_selectionRect = NULL;
    // if(!m_draggingSelRect)
    //   m_graph.clearSelection()
    m_manipulationMode = ManipulationMode_None;
  }
  else if(m_manipulationMode == ManipulationMode_Pan || m_manipulationMode == ManipulationMode_Zoom)
  {
    setCursor(Qt::ArrowCursor);
    m_manipulationMode = ManipulationMode_None;
  }
  else
    QGraphicsWidget::mouseMoveEvent(event);
}

void MainPanel::performZoom(
  float newZoomState,
  QPointF zoomCenter
  )
{
  if ( newZoomState > s_maxZoom )
    newZoomState = s_maxZoom;
  else if ( newZoomState < s_minZoom )
    newZoomState = s_minZoom;

  QPointF oldItemGroupPos = mapToItem( m_itemGroup, zoomCenter );
  float oldScale = m_itemGroup->scale();
  m_itemGroup->setScale( newZoomState );
  QPointF newItemPos = mapFromItem( m_itemGroup, oldItemGroupPos );
  m_itemGroup->setScale( oldScale );
  QPointF delta = newItemPos - zoomCenter;

  m_graph->controller()->zoomCanvas( newZoomState );
  m_graph->controller()->panCanvas( canvasPan() - delta );
}

void MainPanel::wheelEvent(QGraphicsSceneWheelEvent * event)
{
  if ( m_manipulationMode == ManipulationMode_None )
  {
    float zoomFactor = 1.0f + float(event->delta()) * m_mouseWheelZoomRate;
    m_lastPanPoint = event->pos();
    performZoom( m_mouseWheelZoomState * zoomFactor, event->pos() );
  }
}

void MainPanel::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
  if ( windowFrameRect() != m_boundingRect )
  {
    prepareGeometryChange();
    m_boundingRect = windowFrameRect();
  }

  // QRectF rect = windowFrameRect();
  // m_backGround->resize(rect.width(), rect.height());
  QGraphicsWidget::paint(painter, option, widget);
}
