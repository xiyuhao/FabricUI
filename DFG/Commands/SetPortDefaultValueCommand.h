// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_SETPORTDEFAULTVALUE_COMMAND__
#define __UI_SETPORTDEFAULTVALUE_COMMAND__

#include "BaseDFGCommand.h"

namespace FabricUI {
namespace DFG {

class SetPortDefaultValueCommand : public BaseDFGCommand 
{
  public:
    /**
      SetPortDefaultValueCommand sets the value of a DFG port.

      Args:
      - execPath (String): Absolute path of the DFGExec.
      - portName (String): Name of the port.
      - portValue (RTVal): Value to set, must be of the same that the port type.
      - isUndoable (Boolean): If true, the command is undoable.
    */
    
    SetPortDefaultValueCommand();

    virtual ~SetPortDefaultValueCommand();

    /// Implementation of BaseCommand.
    bool canUndo();
 
    /// Implementation of BaseCommand.
    virtual bool doIt();

    /// Implementation of BaseScriptableCommand.
    virtual QString getHelp();
};

} // namespace DFG
} // namespace FabricUI

#endif // __UI_SETPORTDEFAULTVALUE_COMMAND__
