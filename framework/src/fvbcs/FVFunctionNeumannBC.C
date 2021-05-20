//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctionNeumannBC.h"
#include "Function.h"

registerMooseObject("MooseApp", FVFunctionNeumannBC);

InputParameters
FVFunctionNeumannBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Neumann boundary condition for finite volume method.");
  params.addParam<Real>("factor",
                        1.,
                        "A factor for multiplying the function. This could be useful for flipping "
                        "the sign of the function for example based off the normal");
  params.addRequiredParam<FunctionName>("function", "The value of the flux crossing the boundary.");
  return params;
}

FVFunctionNeumannBC::FVFunctionNeumannBC(const InputParameters & parameters)
  : FVFluxBC(parameters), _function(getFunction("function")), _factor(getParam<Real>("factor"))
{
}

ADReal
FVFunctionNeumannBC::computeQpResidual()
{
  return -_factor * _function.value(_t, _face_info->faceCentroid());
}
