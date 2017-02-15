/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMACTION_H
#define STATESIMACTION_H

#include "StateSimBase.h"
#include <string>

//Forward Declarations
class StateSimModel;

class StateSimAction : public StateSimBase
{
public:
  /**
   * This is the main constructor for the base class.
   * @param main_model - the model this action belongs to
   * @param name - name of the item
   * @param act_type - type of action passed in by the child constructor
   */
  StateSimAction(StateSimModel & main_model, const std::string & name, const ACTION_TYPE_ENUM & act_type);
  ACTION_TYPE_ENUM getActType() { return _act_type; };

  //TODO
  //getDerivedJSON
  //getJSON
  //deserializeDerived
  //for gui lookupRelatedItems

protected:
  ACTION_TYPE_ENUM _act_type;


};

#endif
