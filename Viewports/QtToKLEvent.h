#ifndef __QT2OKLEVENT_H__
#define __QT2OKLEVENT_H__

#include <QObject>
#include <QEvent>
#include <QPoint>

#include <FabricCore.h>

FabricCore::RTVal QtToKLMousePosition(QPoint pos, FabricCore::Client const& client, FabricCore::RTVal viewport, bool swapAxis = false);

FabricCore::RTVal QtToKLEvent(QEvent *event, FabricCore::Client const& client, FabricCore::RTVal viewport, char const *hostName, bool swapAxis = false);

#endif
