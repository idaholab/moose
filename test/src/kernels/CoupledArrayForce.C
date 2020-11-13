//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledArrayForce.h"

#include "MooseVariable.h"

registerMooseObject("MooseTestApp", CoupledArrayForce);

InputParameters
CoupledArrayForce::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addRequiredCoupledVar("v", "The coupled array variable which provides the force");
  params.addRequiredParam<RealEigenVector>(
      "coef", "Coefficent ($\\sigma$) multiplier for the coupled force term.");
  params.addClassDescription("Implements a source term proportional to the value of a coupled "
                             "array variable. Weak form: $(u^\\ast, -\\vec{\\sigma} \\vec{v})$.");
  return params;
}

CoupledArrayForce::CoupledArrayForce(const InputParameters & parameters)
  : Kernel(parameters),
    _v_var(coupled("v")),
    _v(coupledArrayValue("v")),
    _coef(getParam<RealEigenVector>("coef"))
{
  if (_var.number() == _v_var)
    mooseError("Coupled variable 'v' needs to be different from 'variable' with CoupledForce, "
               "consider using Reaction or somethig similar");
  if (getArrayVar("v", 0)->count() != _coef.size())
    mooseError("'coef' size in CoupledArrayForce is inconsistent with the number of components of "
               "the coupled array variable");
}

Real
CoupledArrayForce::computeQpResidual()
{
  return -(_coef.transpose() * _v[_qp]).sum() * _test[_i][_qp];
}

RealEigenVector
CoupledArrayForce::computeQpOffDiagJacobianArray(const ArrayMooseVariable & jvar)
{
  if (jvar.number() == _v_var)
    return _coef * (-_phi[_j][_qp] * _test[_i][_qp]);
  else
    return RealEigenVector::Zero(jvar.count());
}
