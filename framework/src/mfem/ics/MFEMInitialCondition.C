//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMInitialCondition.h"
#include <mfem.hpp>

InputParameters
MFEMInitialCondition::validParams()
{
  auto params = MFEMGeneralUserObject::validParams();
  params.addRequiredParam<VariableName>("variable",
                                        "The variable to apply the initial condition on.");
  params.registerBase("InitialCondition");
  // We cannot generally execute this at construction time since the coefficient may be based on a
  // MOOSE function which is not itself setup until its initialSetup is called. UserObject initial
  // execution occurs after function initialSetup
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

MFEMInitialCondition::MFEMInitialCondition(const InputParameters & params)
  : MFEMGeneralUserObject(params)
{
}

#endif
