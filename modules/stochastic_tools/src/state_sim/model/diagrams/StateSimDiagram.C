/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimDiagram.h"
#include "StateSimBase.h"
#include "StateSimState.h"
#include "StateSimModel.h"
#include <string>

StateSimDiagram::StateSimDiagram(StateSimModel & main_model, const std::string & name, DIAGRAM_TYPE_ENUM diag_type)
  : StateSimBase(main_model.nextID(STATESIM_TYPE::DIAGRAM), name),
    _diag_type(diag_type)
{
}

void
StateSimDiagram::addState(StateSimState & add_state)
{
  auto ret = _states.emplace(std::pair<int, StateSimState *>(add_state.id(), &add_state));
  if (ret.second == false)
  {
    //todo log if state was already added.
  }
}
