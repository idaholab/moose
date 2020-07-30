//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryTimeDerivative.h"

registerMooseObject("GeochemistryApp", GeochemistryTimeDerivative);

InputParameters
GeochemistryTimeDerivative::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addCoupledVar("porosity", 1.0, "Porosity");
  params.addClassDescription(
      "Kernel describing porosity * d(concentration)/dt, where porosity is an AuxVariable (or just "
      "a real number) and concentration is the 'variable' for this Kernel.  This Kernel should not "
      "be used if porosity is time-dependent.  Mass lumping is employed for numerical stability");
  return params;
}

GeochemistryTimeDerivative::GeochemistryTimeDerivative(const InputParameters & parameters)
  : TimeKernel(parameters),
    _nodal_u_dot(_var.dofValuesDot()),
    _nodal_du_dot_du(_var.dofValuesDuDotDu()),
    _porosity(coupledValue("porosity"))
{
}

Real
GeochemistryTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _porosity[_qp] * _nodal_u_dot[_i];
}

Real
GeochemistryTimeDerivative::computeQpJacobian()
{
  if (_i == _j)
    return _test[_i][_qp] * _porosity[_qp] * _nodal_du_dot_du[_j];
  else
    return 0.0;
}
