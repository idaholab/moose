//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrossTermBarrierFunctionBase.h"

InputParameters
CrossTermBarrierFunctionBase::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("function_name", "g", "actual name for g(eta_i)");
  MooseEnum g_order("SIMPLE=0 LOW", "SIMPLE");
  params.addParam<MooseEnum>("g_order", g_order, "Polynomial order of the barrier function g(eta)");
  params.addRequiredCoupledVar("etas", "eta_i order parameters, one for each h");
  params.addRequiredParam<std::vector<Real>>("W_ij",
                                             "Terms controlling barrier height set W=1 in "
                                             "DerivativeMultiPhaseMaterial for these to "
                                             "apply");
  return params;
}

CrossTermBarrierFunctionBase::CrossTermBarrierFunctionBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _function_name(getParam<std::string>("function_name")),
    _g_order(getParam<MooseEnum>("g_order")),
    _W_ij(getParam<std::vector<Real>>("W_ij")),
    _num_eta(coupledComponents("etas")),
    _eta_names(coupledNames("etas")),
    _eta(coupledValues("etas")),
    _prop_g(declareProperty<Real>(_function_name)),
    _prop_dg(_num_eta),
    _prop_d2g(_num_eta)
{
  // if Vector W_ij is not the correct size to fill the matrix give error
  if (_num_eta * _num_eta != _W_ij.size())
    paramError("W_ij",
               "Size of W_ij does not match (number of etas)^2. Supply W_ij of correct size.");

  // error out if the W_ij diagonal values are not zero
  for (unsigned int i = 0; i < _num_eta; ++i)
    if (_W_ij[_num_eta * i + i] != 0)
      paramError("W_ij", "Set on-diagonal values of W_ij to zero.");

  // declare g derivative properties, fetch eta values
  for (unsigned int i = 0; i < _num_eta; ++i)
    _prop_d2g[i].resize(_num_eta);

  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    _prop_dg[i] = &declarePropertyDerivative<Real>(_function_name, _eta_names[i]);
    for (unsigned int j = i; j < _num_eta; ++j)
    {
      _prop_d2g[i][j] = _prop_d2g[j][i] =
          &declarePropertyDerivative<Real>(_function_name, _eta_names[i], _eta_names[j]);
    }
  }
}

void
CrossTermBarrierFunctionBase::computeQpProperties()
{
  // Initialize properties to zero before accumulating
  _prop_g[_qp] = 0.0;
  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    (*_prop_dg[i])[_qp] = 0.0;
    (*_prop_d2g[i][i])[_qp] = 0.0;
  }
}
