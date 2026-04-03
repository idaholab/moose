//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMInitialCondition.h"

InputParameters
MFEMInitialCondition::validParams()
{
  auto params = MFEMExecutedObject::validParams();
  params.addRequiredParam<VariableName>("variable",
                                        "The variable to apply the initial condition on.");
  params.registerBase("InitialCondition");
  params.registerSystemAttributeName("MFEMInitialCondition");
  params.addClassDescription(
      "Base class for objects that set the initial condition on an MFEM variable.");
  // We cannot generally execute this at construction time since the coefficient may be based on a
  // MOOSE function which is not itself setup until its initialSetup is called. MFEM initial
  // conditions therefore run in the explicit executed-object pass on EXEC_INITIAL.
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

MFEMInitialCondition::MFEMInitialCondition(const InputParameters & params)
  : MFEMExecutedObject(params)
{
}

std::set<std::string>
MFEMInitialCondition::producedVariableNames() const
{
  return {getParam<VariableName>("variable")};
}

#endif
