// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#include <iostream>
#include <FabricUI/GraphView/Controller.h>
#include <FabricUI/GraphView/Graph.h>
#include <FabricUI/GraphView/Node.h>
#include <FabricUI/GraphView/BackDropNode.h>
#include <FabricUI/GraphView/NodeBubble.h>
#include <FabricUI/GraphView/Pin.h>
#include <FabricUI/GraphView/Port.h>
#include <FabricUI/GraphView/Connection.h>
#include <FabricUI/GraphView/ConnectionTarget.h>

using namespace FabricUI::GraphView;
using namespace FabricServices::Commands;

Controller::Controller(Graph * parent)
{
  m_graph = parent;
  if(m_graph)
    m_graph->setController(this);
  m_compound = NULL;
  m_interactionBracket = 0;
}

Controller::~Controller()
{
  if(m_compound)
    delete(m_compound);
}

Graph * Controller::graph()
{
  return m_graph;
}

const Graph * Controller::graph() const
{
  return m_graph;
}

void Controller::setGraph(Graph * graph)
{
  m_graph = graph;
}

bool Controller::beginInteraction()
{
  if(m_interactionBracket == 0)
  {
    if(m_compound == NULL)
      m_compound = new CompoundCommand();
  }
  return m_interactionBracket++ == 0;
}

bool Controller::endInteraction()
{
  if(m_interactionBracket > 0)
  {
    m_interactionBracket--;
    if(m_interactionBracket == 0)
    {
      if(m_compound)
      {
        delete m_compound;
        m_compound = NULL;
      }
      return true;
    }
  }
  return false;
}

bool Controller::selectNode(Node * node, bool state)
{
  if(node->selected() != state)
  {
    node->setSelected(state);
    return true;
  }
  return false;
}

bool Controller::clearSelection()
{
  if(!m_graph)
    return false;
  std::vector<Node*> nodes = m_graph->selectedNodes();
  for(size_t i=0;i<nodes.size();i++)
  {
    nodes[i]->setSelected(false);
  }
  return nodes.size() > 0;
}

bool Controller::gvcDoRemoveConnections(const std::vector<Connection*> & conns)
{
  std::vector<ConnectionTarget*> srcs;
  std::vector<ConnectionTarget*> dsts;
  for (size_t i=0;i<conns.size();i++)
    if (conns[i])
    { srcs.push_back(conns[i]->src());
      dsts.push_back(conns[i]->dst()); }
  return gvcDoRemoveConnections(srcs, dsts);
}

bool Controller::zoomCanvas(float zoom)
{
  if(!m_graph)
    return false;
  m_graph->mainPanel()->setCanvasZoom(zoom, true);
  return true;
}

bool Controller::panCanvas(QPointF pan)
{
  if(!m_graph)
    return false;
  m_graph->mainPanel()->setCanvasPan(pan, true);
  return true;
}

bool Controller::frameAndFitNodes( FTL::ArrayRef<Node *> nodes )
{
  if(!m_graph)
    return false;
  if(nodes.size() == 0)
    return false;
  MainPanel *mainPanel = m_graph->mainPanel();

  // Get the boudingRect of the nodes
  QRectF nodesBoundingRect_ItemGroup;
  for ( size_t i = 0; i < nodes.size(); ++i )
  {
    Node *node = nodes[i];
    QRectF nodeBoundingRect = node->boundingRect();
    QPointF nodeTopLeftPos = node->topLeftGraphPos();
    nodesBoundingRect_ItemGroup |=
      nodeBoundingRect.translated( nodeTopLeftPos );
  }

  QRectF mainPanelBoundingRect_MainPanel = mainPanel->boundingRect();

  // Compute the appropriate zoom
  // Zoom out a bit more, otherwise nodes will lies on the panel border
  float zoom = std::min(
    mainPanelBoundingRect_MainPanel.width()
      / ( nodesBoundingRect_ItemGroup.width() * 1.1 ),
    mainPanelBoundingRect_MainPanel.height()
      / ( nodesBoundingRect_ItemGroup.height() * 1.1 )
    );

  zoomCanvas( std::max( 0.000001f, std::min( zoom, 1.0f ) ) );
  panCanvas(
      mainPanelBoundingRect_MainPanel.center()
    - nodesBoundingRect_ItemGroup.center() * mainPanel->canvasZoom()
    );

  return true;
}

bool Controller::frameSelectedNodes()
{
  return frameAndFitNodes(m_graph->selectedNodes());
}

bool Controller::frameAllNodes()
{
  return frameAndFitNodes(m_graph->nodes());
}

bool Controller::nodesAreVisible( FTL::ArrayRef<Node *> nodes )
{
  if(!m_graph)
    return false;
  if(nodes.size() == 0)
    return false;
  MainPanel *mainPanel = m_graph->mainPanel();

  // get the boundingRect of the nodes.
  QRectF nodesBoundingRect_ItemGroup;
  for ( size_t i = 0; i < nodes.size(); ++i )
  {
    Node *node = nodes[i];
    QRectF nodeBoundingRect = node->boundingRect();
    QPointF nodeTopLeftPos = node->topLeftGraphPos();
    nodesBoundingRect_ItemGroup |=
      nodeBoundingRect.translated( nodeTopLeftPos );
  }

  // consider pan and zoom.
  QPointF pnt1 = nodesBoundingRect_ItemGroup.topLeft();
  QPointF pnt2 = nodesBoundingRect_ItemGroup.bottomRight();
  pnt1 *= mainPanel->canvasZoom();
  pnt1 += mainPanel->canvasPan();
  pnt2 *= mainPanel->canvasZoom();
  pnt2 += mainPanel->canvasPan();
  nodesBoundingRect_ItemGroup.setTopLeft(pnt1);
  nodesBoundingRect_ItemGroup.setBottomRight(pnt2);

  // return true if boundingRect is fully contained in the view rect.
  return mainPanel->boundingRect().contains(nodesBoundingRect_ItemGroup);
}

bool Controller::allNodesAreVisible()
{
  return nodesAreVisible(m_graph->nodes());
}

void Controller::collapseNodes(int state, const std::vector<Node*> & nodes) {
  for(size_t i=0;i<nodes.size();i++)
    nodes[i]->setCollapsedState((Node::CollapseState)state);
}

void Controller::collapseSelectedNodes(int state)
{
  collapseNodes(state, m_graph->selectedNodes());
}

void Controller::onNodeHeaderButtonTriggered(FabricUI::GraphView::NodeHeaderButton * button)
{
  if(button->name() == "node_collapse")
  {
    int state = (button->state() + 1) % Node::CollapseState_NumStates;
    button->header()->node()->setCollapsedState((Node::CollapseState)state);
  }
}

bool Controller::canConnectTo(
  char const *pathA,
  char const *pathB,
  std::string &failureReason
  ) const
{
  return true;
}
