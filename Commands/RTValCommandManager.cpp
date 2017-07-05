//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#include "CommandRegistry.h"
#include "RTValCommandManager.h"
#include <FabricUI/Util/RTValUtil.h>
#include "BaseRTValScriptableCommand.h"
#include <FabricUI/Application/FabricException.h>
#include <FabricUI/Commands/PathValueResolverRegistry.h>

using namespace FabricUI;
using namespace Util;
using namespace Commands;
using namespace FabricCore;
using namespace Application;
 
RTValCommandManager::RTValCommandManager() 
  : CommandManager()
{
}

RTValCommandManager::~RTValCommandManager() 
{
}
 
BaseCommand* RTValCommandManager::createCommand(
  QString const&cmdName, 
  QMap<QString, QString> const&args, 
  bool doCmd,
  int canMergeID)
{
  return CommandManager::createCommand(
    cmdName, 
    args, 
    doCmd, 
    canMergeID);
}

BaseCommand* RTValCommandManager::createCommand(
  QString const&cmdName, 
  QMap<QString, RTVal> const&args, 
  bool doCmd,
  int canMergeID)
{
  FABRIC_CATCH_BEGIN();

  BaseCommand *cmd = CommandRegistry::getCommandRegistry()->createCommand(
    cmdName);

  if(args.size() > 0) 
    checkRTValCommandArgs(cmd, args);

  if(doCmd) 
    doCommand(cmd, canMergeID);

  return cmd;

  FABRIC_CATCH_END("RTValCommandManager::createCommand");

  return 0;
}

void RTValCommandManager::checkRTValCommandArgs(
  BaseCommand *cmd,
  QMap<QString, RTVal> const&args)
{ 
  BaseRTValScriptableCommand* scriptCmd = qobject_cast<BaseRTValScriptableCommand*>(cmd);

  if(!scriptCmd) 
    FabricException::Throw(
      "RTValCommandManager::checkRTValCommandArgs",
      "BaseCommand '" + cmd->getName() + "' is created with args, " + 
      "but is not implementing the BaseRTValScriptableCommand interface"
      );

  // Sets the rtval args
  QMapIterator<QString, RTVal> ite(args);
  while(ite.hasNext()) 
  {
    ite.next();
    RTVal arg = ite.value();

    if(RTValUtil::getType(arg) == "PathValue")
      scriptCmd->setRTValArg(ite.key(), arg);
    else
      scriptCmd->setRTValArgValue(ite.key(), RTValUtil::toKLRTVal(arg));
  }

  scriptCmd->validateSetArgs();
}

void RTValCommandManager::preDoCommand(
  BaseCommand* cmd)
{
  BaseRTValScriptableCommand* scriptCmd = qobject_cast<BaseRTValScriptableCommand*>(cmd);
  if(!scriptCmd) return;

  FABRIC_CATCH_BEGIN();

  QString key;
  foreach(key, scriptCmd->getArgKeys())
  {
    if( scriptCmd->hasArgFlag(key, CommandArgFlags::IN_ARG) ||
        scriptCmd->hasArgFlag(key, CommandArgFlags::IO_ARG) )
    {
      RTVal pathValue = scriptCmd->getRTValArg(key);
      if(PathValueResolverRegistry::getRegistry()->knownPath(pathValue))
      {
        PathValueResolverRegistry::getRegistry()->getValue(pathValue);
        scriptCmd->setRTValArg(key, pathValue);
      }
    }
  }

  FABRIC_CATCH_END("RTValCommandManager::preDoCommand");
}

void RTValCommandManager::postDoCommand(
  BaseCommand* cmd)
{
  BaseRTValScriptableCommand* scriptCmd = qobject_cast<BaseRTValScriptableCommand*>(cmd);
  if(!scriptCmd) return;

  FABRIC_CATCH_BEGIN();

  QString key;
  foreach(key, scriptCmd->getArgKeys())
  {         
    if( scriptCmd->hasArgFlag(key, CommandArgFlags::OUT_ARG) ||
        scriptCmd->hasArgFlag(key, CommandArgFlags::IO_ARG) )
    {
      RTVal pathValue = scriptCmd->getRTValArg(key);
      if(PathValueResolverRegistry::getRegistry()->knownPath(pathValue))
        PathValueResolverRegistry::getRegistry()->setValue(
          pathValue);
    }
  }

  FABRIC_CATCH_END("RTValCommandManager::postDoCommand");
}
