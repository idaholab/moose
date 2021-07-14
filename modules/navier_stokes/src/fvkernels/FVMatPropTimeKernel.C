//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMatPropTimeKernel.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", FVMatPropTimeKernel);

InputParameters
FVMatPropTimeKernel::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Returns a material property which should correspond to a time derivative.");
  params.addRequiredParam<MaterialPropertyName>(
      "mat_prop_time_derivative", "The material property containing the time derivative.");
  return params;
}

FVMatPropTimeKernel::FVMatPropTimeKernel(const InputParameters & parameters)
  : FVTimeKernel(parameters),
    _mat_prop_time_derivative(getADMaterialProperty<Real>("mat_prop_time_derivative"))
{
}

ADReal
FVMatPropTimeKernel::computeQpResidual()
{
  return _mat_prop_time_derivative[_qp];
}
