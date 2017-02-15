/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimAction.h"
#include "StateSimBase.h"
#include "StateSimModel.h"
#include <string>

StateSimAction::StateSimAction(StateSimModel & main_model, const std::string & name, const ACTION_TYPE_ENUM & act_type)
  : StateSimBase(main_model.nextID(STATESIM_TYPE::ACTION), name),
    _act_type(act_type)
{
}
