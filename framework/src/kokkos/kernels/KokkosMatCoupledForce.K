//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMatCoupledForce.h"

#include "MooseVariable.h"

registerKokkosResidualObject("MooseApp", KokkosMatCoupledForce);

InputParameters
KokkosMatCoupledForce::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addClassDescription(
      "Implements a forcing term RHS of the form PDE = RHS, where RHS = Sum_j c_j * m_j * v_j. "
      "c_j, m_j, and v_j are provided as real coefficients, material properties, and coupled "
      "variables, respectively.");
  params.addRequiredCoupledVar("v", "The coupled variables which provide the force");
  params.addParam<std::vector<Real>>(
      "coef", "Coefficents ($\\sigma$) multiplier for the coupled force term.");
  params.addParam<std::vector<MaterialPropertyName>>("material_properties",
                                                     "The coupled material properties.");
  return params;
}

KokkosMatCoupledForce::KokkosMatCoupledForce(const InputParameters & parameters)
  : Kernel(parameters),
    _n_coupled(coupledComponents("v")),
    _coupled_props(isParamValid("material_properties")),
    _v_var(coupledIndices("v")),
    _v(kokkosCoupledValues("v")),
    _coef(isParamValid("coef") ? getParam<std::vector<Real>>("coef")
                               : std::vector<Real>(_n_coupled, 1))
{
  for (const auto j : make_range(_n_coupled))
  {
    _v_var_to_index[_v_var[j]] = j;

    for (const auto var : variables())
      if (var->number() == _v_var[j])
        paramError("v",
                   "Coupled variable 'v' needs to be different from variable, consider using "
                   "KokkosReaction or something similar");
  }

  _v_var_to_index.copyToDevice();

  if (isParamValid("coef") && _coef.size() != _n_coupled)
    paramError("coef", "Size of coef must be equal to size of v");

  if (_coupled_props)
  {
    _mat_props.create(_n_coupled);
    const auto & names = getParam<std::vector<MaterialPropertyName>>("material_properties");
    if (names.size() != _n_coupled)
      paramError("material_properties", "Size must be equal to number of coupled variables");
    for (const auto j : make_range(_n_coupled))
      _mat_props[j] = getKokkosMaterialPropertyByName<Real>(names[j]);
  }
}
