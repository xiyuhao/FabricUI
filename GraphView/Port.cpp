// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#include <FabricUI/GraphView/FixedPort.h>
#include <FabricUI/GraphView/Graph.h>
#include <FabricUI/GraphView/GraphConfig.h>
#include <FabricUI/GraphView/InstBlockPort.h>
#include <FabricUI/GraphView/Pin.h>
#include <FabricUI/GraphView/Port.h>
#include <FabricUI/GraphView/SidePanel.h>

#include <QGraphicsLinearLayout>

using namespace FabricUI::GraphView;

Port::Port(
  SidePanel * parent,
  FTL::StrRef name,
  PortType portType,
  FTL::StrRef dataType,
  QColor color,
  FTL::StrRef label
  )
  : ConnectionTarget( parent->itemGroup() )
  , m_sidePanel( parent )
  , m_name( name )
  , m_labelCaption( !label.empty()? label: name )
  , m_allowEdits( true )
{
  // if(parent->graph()->path().length() > 0)
  //   m_path = parent->graph()->path() + parent->graph()->config().pathSep + m_path;
  m_portType = portType;
  m_dataType = dataType;
  m_color = color;
  m_index = 0;

  init();
}

void Port::init()
{
  setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));

  const GraphConfig & config = graph()->config();

  QGraphicsLinearLayout * layout = new QGraphicsLinearLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(config.sidePanelPortLabelSpacing);
  layout->setOrientation(Qt::Horizontal);
  setLayout(layout);

  m_label = new PortLabel(
    this,
    QSTRING_FROM_STL_UTF8(m_labelCaption),
    config.sidePanelFontColor,
    config.sidePanelFontHighlightColor,
    config.sidePanelFont
    );
  m_circle = new PinCircle(this, m_portType, color());

  if(m_portType == PortType_Input)
  {
    layout->addItem(m_circle);
    layout->setAlignment(m_circle, Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addItem(m_label);
    layout->setAlignment(m_label, Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addStretch(1);
  }
  else if(m_portType == PortType_Output)
  {
    layout->addStretch(1);
    layout->addItem(m_label);
    layout->setAlignment(m_label, Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addItem(m_circle);
    layout->setAlignment(m_circle, Qt::AlignHCenter | Qt::AlignVCenter);
  }
}

void Port::disableEdits()
{
  m_allowEdits = false;
  m_label->setEditable( false );
}

Graph *Port::graph()
{
  return sidePanel()->graph();
}

Graph const *Port::graph() const
{
  return sidePanel()->graph();
}

void Port::setName( FTL::CStrRef name )
{
  bool labelUsesName = m_name == m_labelCaption;
  m_name = name;
  if(labelUsesName)
    setLabel(name.c_str());
  else
    update();

  emit contentChanged();
}

char const * Port::label() const
{
  return m_labelCaption.c_str();
}

void Port::setLabel(char const * n)
{
  m_labelCaption = n;
  m_label->setText(QSTRING_FROM_STL_UTF8(m_labelCaption));
  update();
}

QColor Port::color() const
{
  return m_color;
}

PortType Port::portType() const
{
  return m_portType;
}

void Port::setDataType(FTL::CStrRef dataType)
{
  m_dataType = dataType;
  setToolTip(m_dataType.c_str());
}

void Port::setColor(QColor color)
{
  m_color = color;
  m_circle->setColor(m_color);
}

bool Port::highlighted() const
{
  return m_highlighted;
}

void Port::setHighlighted(bool state)
{
  if(m_highlighted != state)
  {
    m_circle->setHighlighted(state);
    m_label->setHighlighted(state);
    m_highlighted = state;
  }
}

bool Port::canConnectTo(
  ConnectionTarget * other,
  std::string &failureReason
  ) const
{
  switch(other->targetType())
  {
    case TargetType_Pin:
    {
      Pin * otherPin = (Pin *)other;
      if ( this == other
        || portType() == PortType_Input
        || otherPin->portType() == PortType_Output )
        return false;
      return m_sidePanel->graph()->controller()->canConnectTo(
        path().c_str(),
        otherPin->path().c_str(),
        failureReason
        );
    }
    case TargetType_Port:
    {
      Port * otherPort = (Port *)other;
      if ( this == otherPort
        || portType() == PortType_Input
        || otherPort->portType() == PortType_Output )
        return false;
      if(path() == otherPort->path())
        return false;
      return m_sidePanel->graph()->controller()->canConnectTo(
        path().c_str(),
        otherPort->path().c_str(),
        failureReason
        );
    }
    case TargetType_FixedPort:
    {
      FixedPort *otherFixedPort = (FixedPort *)other;
      if ( portType() == PortType_Input
        || otherFixedPort->portType() == PortType_Output )
        return false;
      if(path() == otherFixedPort->path())
        return false;
      return m_sidePanel->graph()->controller()->canConnectTo(
        path().c_str(),
        otherFixedPort->path().c_str(),
        failureReason
        );
    }
    case TargetType_InstBlockPort:
    {
      InstBlockPort *otherInstBlockPort = (InstBlockPort *)other;
      if ( portType() == PortType_Input
        || otherInstBlockPort->portType() == PortType_Output )
        return false;
      if(path() == otherInstBlockPort->path())
        return false;
      return m_sidePanel->graph()->controller()->canConnectTo(
        path().c_str(),
        otherInstBlockPort->path().c_str(),
        failureReason
        );
    }
    case TargetType_NodeHeader:
    case TargetType_InstBlockHeader:
      return true;
    default:
      return false;
  }
}

QPointF Port::connectionPos(PortType pType) const
{
  return m_circle->centerInSceneCoords();
}

void Port::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
  QMenu * menu = graph()->getPortContextMenu( this );
  if ( menu )
  {
    menu->exec( QCursor::pos() );
    menu->setParent( NULL );
    menu->deleteLater();
  }
}

std::string Port::path() const
{
  return m_name;
}
