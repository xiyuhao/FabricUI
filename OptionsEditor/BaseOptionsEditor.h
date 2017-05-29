//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#ifndef __UI_BASE_OPTIONE_DITOR__
#define __UI_BASE_OPTIONE_DITOR__

#include <QUndoStack>
#include <FabricUI/ValueEditor/VETreeWidget.h>

namespace FabricUI {
namespace OptionsEditor {

class OptionsModel;
class OptionsDictModel;

class BaseOptionsEditor : public ValueEditor::VETreeWidget
{
  /**
    BaseOptionsEditor specializes ValueEditor::VETreeWidget
     to edit the content of OptionsDictModel dictionary.
  */  
  Q_OBJECT

  public:
    BaseOptionsEditor( 
      QUndoStack *undoStack 
      );

    virtual ~BaseOptionsEditor();

  public slots:
    /// Update when the dictionary content changed.
    virtual void updateOptions();

    /// Update when a dictionary value changed.
    void onValueChanged();

    /// Update when a dictionary value is committed.
    void onValueCommitted(QUndoCommand *);

  signals:
    /// Emited when the value of one option has changed.
    void valueChanged();

  protected:
    QUndoStack *m_undoStack;
    OptionsDictModel* m_model;
};

} // namespace OptionsEditor 
} // namespace FabricUI

#endif // __UI_BASE_OPTIONE_DITOR__