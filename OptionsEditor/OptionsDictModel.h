//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#ifndef __UI_OPTION_DICT_MODEL__
#define __UI_OPTION_DICT_MODEL__

#include <QSettings>
#include <FabricCore.h>
#include "BaseOptionsEditor.h"
#include <FabricUI/ValueEditor/BaseModelItem.h>

namespace FabricUI {
namespace OptionsEditor {

class OptionsDictModel : public ValueEditor::BaseModelItem 
{
  /**
    OptionsDictModel specializes ValueEditor::BaseModelItem and gather
    OptionsModels in a key-value pair dictionary [key(String), value(RTVal)].
  */  
  Q_OBJECT
  
  public:
    OptionsDictModel(
      const std::string &name,
      FabricCore::RTVal dict,
      QSettings* settings,
      const std::string &namePath,
      BaseOptionsEditor* editor
      );

    virtual ~OptionsDictModel();

    /// Implementation of BaseModelItem
    virtual FTL::CStrRef getName();

    /// Implementation of BaseModelItem
    virtual int getNumChildren();

    /// Implementation of BaseModelItem
    virtual BaseModelItem* getChild(
      FTL::StrRef childName, 
      bool doCreate
      );

    /// Implementation of BaseModelItem
    virtual BaseModelItem* getChild(
      int index, 
      bool doCreate
      );
    
    /// Implementation of BaseModelItem
    virtual bool hasDefault();
    
    /// Implementation of BaseModelItem
    virtual void resetToDefault();

  protected:
    /// Name of the dictionary.
    std::string m_name;
    /// Path of the dictionary.
    std::string m_namePath;
    /// Dictionary [key, value]
    std::map<std::string, BaseModelItem*> m_children;
    /// Lists of the options' key.
    std::vector<std::string> m_keys;
};

} // namespace OptionsEditor 
} // namespace FabricUI

#endif // __UI_OPTION_DICT_MODEL__