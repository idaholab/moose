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
  params.addRequiredCoupledVar(
      "variable", "The variable whose value will be fitted over the patch of elements.");

  params.addClassDescription("Performs patch recovery using a coupled variable.");

  return params;
}

NodalPatchRecoveryVariable::NodalPatchRecoveryVariable(const InputParameters & params)
  : NodalPatchRecoveryBase(params), _v(coupledValue("variable")), _name(coupledName("variable"))
{
}

Real
NodalPatchRecoveryVariable::computeValue()
{
  return _v[_qp];
}

const VariableName &
NodalPatchRecoveryVariable::variableName() const
{
  return _name;
}
