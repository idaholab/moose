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
  params.addParam<bool>("is_v_array", false, "Whether v is a array variable or not");
  params.addRequiredParam<RealEigenVector>(
      "coef", "Coefficent ($\\sigma$) multiplier for the coupled force term.");
  params.addClassDescription(
      "Implements a source term proportional to the value of a coupled "
      "standard variable. Weak form: $(\\vec{u}^\\ast, -\\vec{\\sigma} v)$.");
  return params;
}

ArrayCoupledForce::ArrayCoupledForce(const InputParameters & parameters)
  : ArrayKernel(parameters),
    _is_v_array(getParam<bool>("is_v_array")),
    _v_var(coupled("v")),
    _v(_is_v_array ? nullptr : &coupledValue("v")),
    _v_array(_is_v_array ? &coupledArrayValue("v") : nullptr),
    _coef(getParam<RealEigenVector>("coef"))
{
  if (_var.number() == _v_var)
    paramError("v",
               "Coupled variable 'v' needs to be different from 'variable' with ArrayCoupledForce, "
               "consider using Reaction or somethig similar");
  if (_is_v_array && getArrayVar("v", 0)->count() != _count)
    paramError("v",
               "Needs to be either a standard variable or an array variable with the same "
               "number of components of 'variable'");
}

void
ArrayCoupledForce::computeQpResidual(RealEigenVector & residual)
{
  if (_is_v_array)
    residual = -_coef.cwiseProduct((*_v_array)[_qp]) * _test[_i][_qp];
  else
    residual = -_coef * (*_v)[_qp] * _test[_i][_qp];
}

RealEigenMatrix
ArrayCoupledForce::computeQpOffDiagJacobian(const MooseVariableFEBase & jvar)
{
  if (jvar.number() == _v_var)
  {
    RealEigenVector v = _coef * (-_phi[_j][_qp] * _test[_i][_qp]);
    if (_is_v_array)
    {
      RealEigenMatrix t = RealEigenMatrix::Zero(_var.count(), _var.count());
      t.diagonal() = v;
      return t;
    }
    else
      return v;
  }
  else
    return RealEigenMatrix::Zero(_var.count(), jvar.count());
}
