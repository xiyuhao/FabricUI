/*
 *  Copyright 2010-2016 Fabric Software Inc. All rights reserved.
 */

#ifndef __UI_SCENEHUB_SHTREEVIEW_H__
#define __UI_SCENEHUB_SHTREEVIEW_H__

#include <QtGui/QMenu>
#include <QtGui/QTreeView>
#include <QtGui/QMouseEvent>
#include <FabricUI/SceneHub/SHStates.h>
#include <FabricUI/SceneHub/SHGLScene.h>
#include "SHBaseTreeView.h"


namespace FabricUI {
namespace SceneHub {

class SHTreeView : public SHBaseTreeView {
  
  /**
    SHTreeView specializes SceneHub::SHBaseTreeView.
    This widgets is used in the DFG::SHDFGCombinedWiget for DCC integration.

    It gives access to the base signals/slots so it can be specialized if needed.
  */

  Q_OBJECT


  public:
    /// Constructor.
    /// \param client A reference to the FabricCore::Client
    /// \param shStates A reference to the FabricUI::SceneHub::SHStates
    /// \param shGLScene A reference to the FabricUI::SceneHub::SHGLScene
    SHTreeView(
      FabricCore::Client client,
      SHStates *shStates, 
      SHGLScene *shGLScene);
        
    /// Detructor.
    virtual ~SHTreeView() {}
    
    /// Implementation of :`QWidget`.
    virtual void mouseDoubleClickEvent(QMouseEvent *event);


  public slots:
    /// Implementation of :`QTreeView`.
    /// Displays the contextual Menu.
    void onCustomContextMenu(const QPoint &point);


  signals:
    /// Emitted when the selection is cleared.
    /// Used to synchronize with the 3D view.
    void selectionCleared();

    /// Emitted when an item is selected.
    /// Used to synchronize with the 3D view.
    void itemSelected(FabricUI::SceneHub::SHTreeItem *item);

    /// Emitted when an item is deselected.
    /// Used to synchronize with the 3D view.
    void itemDeselected(FabricUI::SceneHub::SHTreeItem *item);

    /// Emitted when an item is double-clicked.
    /// Used to edit this item pproperties in the valueEditor.
    void itemDoubleClicked(FabricUI::SceneHub::SHTreeItem *item);


  protected:
    /// Selects/Unselects treeView items.
    /// implementation of : `SceneHub::SHBaseTreeView`
    virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    
    /// Reference to the FabricUI::SceneHub::SHStates
    SHStates *m_shStates;
    /// Reference to the FabricUI::SceneHub::SHGLScene
    SHGLScene *m_shGLScene;

};

} // namespace SceneHub
} // namespace FabricUI

#endif // __UI_SCENEHUB_SHTREEVIEW_H__