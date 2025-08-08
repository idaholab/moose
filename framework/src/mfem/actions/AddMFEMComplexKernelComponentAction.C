//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "AddMFEMComplexKernelComponentAction.h"

registerMooseAction("MooseApp",
                    AddMFEMComplexKernelComponentAction,
                    "add_mfem_complex_kernel_components");

InputParameters
AddMFEMComplexKernelComponentAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription(
      "Add an MFEM AuxKernel to serve as the real or imaginary component of an MFEMComplexKernel.");
  return params;
}

AddMFEMComplexKernelComponentAction::AddMFEMComplexKernelComponentAction(
    const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMComplexKernelComponentAction::act()
{
  if (_name == "real_part" || _name == "imag_part")
  {
    // Finding the string "parent/real_part" or "parent/imag_part" to associate with the object
    std::string action_name = _app.actionWarehouse().getCurrentActionName();
    int second_last_slash = 0;
    int occur = 0;
    for (int i = action_name.length() - 1; i >= 0; --i)
    {
      if (action_name[i] == '/')
        occur += 1;

      if (occur == 2)
      {
        second_last_slash = i;
        break;
      }
    }

    std::string comp_name =
        action_name.substr(second_last_slash + 1, action_name.length() - second_last_slash - 1);

    MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
    if (mfem_problem)
      mfem_problem->addComplexComponentToKernel(_type, comp_name, _moose_object_pars);
  }
}

#endif
