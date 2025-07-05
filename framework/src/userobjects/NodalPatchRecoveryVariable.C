//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalPatchRecoveryVariable.h"

registerMooseObject("MooseApp", NodalPatchRecoveryVariable);

InputParameters
NodalPatchRecoveryVariable::validParams()
{
  InputParameters params = NodalPatchRecoveryBase::validParams();

  params.addRequiredCoupledVar("var", "The scalar variable to recover at nodes");

  return params;
}

NodalPatchRecoveryVariable::NodalPatchRecoveryVariable(const InputParameters & params)
  : NodalPatchRecoveryBase(params), _var(coupledValue("var")), _var_name(coupledName("var"))
{
  setVariableName(_var_name);
}
