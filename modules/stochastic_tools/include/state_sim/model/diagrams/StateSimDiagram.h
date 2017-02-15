/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMDIAGRAM_H
#define STATESIMDIAGRAM_H

#include "StateSimBase.h"
#include <string>
#include <vector>

//Forward Declarations
class StateSimState;
class StateSimModel;

/**
 * Root diagram class, direct creation should only be for Plant Response Digrams.
 */
class StateSimDiagram : public StateSimBase
{
public:
  /**
   * This is the main construter for the user.
   * @param name - name of the item
   * @param diag_type - type of diaram.
   */
  StateSimDiagram(StateSimModel & main_model, const std::string & name, DIAGRAM_TYPE_ENUM diag_type);
  DIAGRAM_TYPE_ENUM getEvType()
  {
    return _diag_type;
  }

  void clear()
  {
    _states.clear();
  };
  /**
   * addState - Add a state to this diagram.
   * @param add_state - state to add
   */
  void addState(StateSimState & add_state);

  //todo
  //getDerivedJSON
  //getJSON
  //LoadObjLinks
  //LookupRelatedItems

protected:
  DIAGRAM_TYPE_ENUM _diag_type;
  std::map<int, StateSimState *> _states;
};

#endif
