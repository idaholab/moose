//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosArrayCoupledForce.h"

registerKokkosResidualObject("MooseApp", KokkosArrayCoupledForce);

InputParameters
KokkosArrayCoupledForce::validParams()
{
  InputParameters params = ArrayKernel::validParams();
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  params.addParam<bool>("is_v_array", false, "Whether v is an array variable or not");
  params.addRequiredParam<RealEigenVector>(
      "coef", "Coefficient ($\\sigma$) multiplier for the coupled force term.");
  params.addClassDescription(
      "Implements a source term proportional to the value of a coupled standard or array "
      "variable. Weak form: $(\\vec{u}^\\ast, -\\vec{\\sigma} v)$.");
  return params;
}

KokkosArrayCoupledForce::KokkosArrayCoupledForce(const InputParameters & parameters)
  : ArrayKernel(parameters),
    _is_v_array(getParam<bool>("is_v_array")),
    _v_var(coupled("v")),
    _v(_is_v_array ? kokkosZeroValue() : kokkosCoupledValue("v")),
    _v_array(_is_v_array ? kokkosCoupledArrayValue("v") : kokkosZeroValue()),
    _coef(getParam<RealEigenVector>("coef"))
{
  if (_var.number() == _v_var)
    paramError("v",
               "Coupled variable 'v' needs to be different from 'variable' with "
               "KokkosArrayCoupledForce.");
  if (_is_v_array && getArrayVar("v", 0)->count() != _count)
    paramError("v",
               "Needs to be either a standard variable or an array variable with the same "
               "number of components as 'variable'");
  if (getParam<RealEigenVector>("coef").size() != _count)
    paramError("coef",
               "Number of 'coef' entries must equal number of variable components (",
               _count,
               ").");
}
