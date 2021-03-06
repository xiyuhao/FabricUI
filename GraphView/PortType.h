// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_GraphView_PortType__
#define __UI_GraphView_PortType__

#include <FabricCore.h>

#include <assert.h>

namespace FabricUI {
namespace GraphView {

enum PortType
{
  PortType_Input,
  PortType_Output,
  PortType_IO
};

inline PortType InvertPortType( PortType portType )
{
  PortType result;
  switch ( portType )
  {
    case GraphView::PortType_Input:
      result = GraphView::PortType_Output;
      break;
    
    case GraphView::PortType_Output:
      result = GraphView::PortType_Input;
      break;
    
    case GraphView::PortType_IO:
      result = GraphView::PortType_IO;
      break;

    default:
      // [andrew 20150730] shouldn't be possible but needed to prevent
      // compiler warning
      assert( false );
      result = GraphView::PortType_Output;
      break;
  }
  return result;
}

inline FabricCore::DFGPortType PortTypeToDFGPortType( PortType portType )
{
  FabricCore::DFGPortType dfgPortType;
  switch ( portType )
  {
    case GraphView::PortType_Input:
      dfgPortType = FabricCore::DFGPortType_Out;
      break;
    
    case GraphView::PortType_Output:
      dfgPortType = FabricCore::DFGPortType_In;
      break;
    
    case GraphView::PortType_IO:
      dfgPortType = FabricCore::DFGPortType_IO;
      break;

    default:
      // [andrew 20150730] shouldn't be possible but needed to prevent
      // compiler warning
      assert( false );
      dfgPortType = FabricCore::DFGPortType_In;
      break;
  }
  return dfgPortType;
}

} // namespace GraphView
} // namespace FabricUI

#endif // __UI_GraphView_PortType__

