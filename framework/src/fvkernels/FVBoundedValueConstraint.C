//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVBoundedValueConstraint.h"

#include "MooseVariableScalar.h"
#include "MooseVariableFV.h"
#include "Assembly.h"

registerMooseObject("MooseApp", FVBoundedValueConstraint);

InputParameters
FVBoundedValueConstraint::validParams()
{
  InputParameters params = FVScalarLagrangeMultiplierConstraint::validParams();
  params.addClassDescription(
      "This class is used to enforce a min or max value for a finite volume variable");
  params.setDocString("phi0", "The min or max bound");
  // Define the min/max enumeration
  MooseEnum type_options("lower_than=0 higher_than=1");
  params.addRequiredParam<MooseEnum>(
      "bound_type", type_options, "Whether a minimum or a maximum bound");
  return params;
}

FVBoundedValueConstraint::FVBoundedValueConstraint(const InputParameters & parameters)
  : FVScalarLagrangeMultiplierConstraint(parameters), _bound_type(getParam<MooseEnum>("bound_type"))
{
}

ADReal
FVBoundedValueConstraint::computeQpResidual()
{
  if (_bound_type == BoundType::LOWER_THAN && _u[_qp] > _phi0)
    return _var(makeElemArg(_current_elem), determineState()) - _phi0;
  else if (_bound_type == BoundType::HIGHER_THAN && _u[_qp] < _phi0)
    return _phi0 - _var(makeElemArg(_current_elem), determineState());
  else
    return 0;
}
