//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "StepUOInterface.h"
#include "StepUserObject.h"

void
StepUOInterface::getStepUserObject(const FEProblemBase & fe_problem,
                                   const StepUserObject *& step_user_object,
                                   const std::string & name)
{
  std::vector<const UserObject *> uos;
  fe_problem.theWarehouse().query().condition<AttribSystem>("UserObject").queryIntoUnsorted(uos);

  std::vector<const StepUserObject *> step_uos;
  for (const auto & uo : uos)
  {
    const StepUserObject * possible_step_uo = dynamic_cast<const StepUserObject *>(uo);
    if (possible_step_uo)
      step_uos.push_back(possible_step_uo);
  }

  if (step_uos.size() > 1)
    mooseError("Your input file has multiple StepUserObjects. MOOSE currently only support one in ",
               name,
               ". \n");
  else if (step_uos.size() == 1)
    mooseInfo(
        "A StepUserObject, has been identified and will be used to drive stepping behavior in ",
        name,
        ".");

  step_user_object = step_uos.size() == 1 ? step_uos[0] : nullptr;
}
