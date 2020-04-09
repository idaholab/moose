//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PrimaryTimeDerivative.h"

registerMooseObject("ChemicalReactionsApp", PrimaryTimeDerivative);

InputParameters
PrimaryTimeDerivative::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  params.addClassDescription("Derivative of primary species concentration wrt time");
  return params;
}

PrimaryTimeDerivative::PrimaryTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters), _porosity(getMaterialProperty<Real>("porosity"))
{
}

Real
PrimaryTimeDerivative::computeQpResidual()
{
  return _porosity[_qp] * TimeDerivative::computeQpResidual();
}

Real
PrimaryTimeDerivative::computeQpJacobian()
{
  return _porosity[_qp] * TimeDerivative::computeQpJacobian();
}
