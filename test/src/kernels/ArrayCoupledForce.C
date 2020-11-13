//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayCoupledForce.h"

#include "MooseVariable.h"

registerMooseObject("MooseTestApp", ArrayCoupledForce);

InputParameters
ArrayCoupledForce::validParams()
{
  InputParameters params = ArrayKernel::validParams();
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  params.addRequiredParam<RealEigenVector>(
      "coef", "Coefficent ($\\sigma$) multiplier for the coupled force term.");
  params.addClassDescription(
      "Implements a source term proportional to the value of a coupled "
      "standard variable. Weak form: $(\\vec{u}^\\ast, -\\vec{\\sigma} v)$.");
  return params;
}

ArrayCoupledForce::ArrayCoupledForce(const InputParameters & parameters)
  : ArrayKernel(parameters),
    _v_var(coupled("v")),
    _v(coupledValue("v")),
    _coef(getParam<RealEigenVector>("coef"))
{
  if (_var.number() == _v_var)
    mooseError("Coupled variable 'v' needs to be different from 'variable' with CoupledForce, "
               "consider using Reaction or somethig similar");
  if (getVar("v", 0)->count() > 1)
    mooseError("We are testing the coupling of a standard variable to an array variable");
}

RealEigenVector
ArrayCoupledForce::computeQpResidual()
{
  return -_coef * (_v[_qp] * _test[_i][_qp]);
}

RealEigenMatrix
ArrayCoupledForce::computeQpOffDiagJacobian(const MooseVariableFEBase & jvar)
{
  if (jvar.number() == _v_var)
    return _coef * (-_phi[_j][_qp] * _test[_i][_qp]);
  else
    return RealEigenMatrix::Zero(_var.count(), jvar.count());
}
