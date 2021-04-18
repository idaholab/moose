//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoeffADVectorTimeDerivative.h"
#include "Function.h"

registerADMooseObject("ElectromagneticsApp", CoeffADVectorTimeDerivative);

InputParameters
CoeffADVectorTimeDerivative::validParams()
{
  InputParameters params = ADVectorTimeDerivative::validParams();
  params.addClassDescription("The time derivative operator with a user-defined coefficient for use "
                             "with vector variables.");
  params.addRequiredParam<FunctionName>("coefficient", "Coefficient function.");
  return params;
}

CoeffADVectorTimeDerivative::CoeffADVectorTimeDerivative(const InputParameters & parameters)
  : ADVectorTimeDerivative(parameters), _coefficient(getFunction("coefficient"))
{
}

ADRealVectorValue
CoeffADVectorTimeDerivative::precomputeQpResidual()
{
  return _coefficient.value(_t, _q_point[_qp]) * ADVectorTimeDerivative::precomputeQpResidual();
}
