//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatDGKernel.h"

// MOOSE includes
#include "MooseVariableFE.h"

registerMooseObject("MooseTestApp", MatDGKernel);

InputParameters
MatDGKernel::validParams()
{
  InputParameters params = DGKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("mat_prop", "This is being tested.");
  return params;
}

MatDGKernel::MatDGKernel(const InputParameters & parameters)
  : DGKernel(parameters), _value(getMaterialProperty<Real>("mat_prop"))
{
}
