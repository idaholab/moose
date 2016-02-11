/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "OPInterfaceBarrierMaterial.h"

template<>
InputParameters validParams<OPInterfaceBarrierMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("function_name", "g", "actual name for g(eta_i)");
  params.addRequiredCoupledVar("etas", "eta_i order parameters, one for each h");
  params.addClassDescription("Free Energy contribution that penalizes more than two order parameters being non-zero");
  return params;
}

OPInterfaceBarrierMaterial::OPInterfaceBarrierMaterial(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _function_name(getParam<std::string>("function_name")),
    _num_eta(coupledComponents("etas")),
    _eta(_num_eta),
    _prop_g(declareProperty<Real>(_function_name)),
    _prop_dg(_num_eta),
    _prop_d2g(_num_eta)
{
  std::vector<std::string> eta_name(_num_eta);
  for (unsigned int i = 0; i < _num_eta; ++i)
  {
      _prop_d2g[i].resize(_num_eta);
  // declare derivative properties, fetch eta values
    eta_name[i] = getVar("etas", i)->name();
  }
  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    _prop_dg[i] = &declarePropertyDerivative<Real>(_function_name, eta_name[i]);
    _eta[i] = &coupledValue("etas", i);
    for (unsigned int j = i; j < _num_eta; ++j)
    {
      _prop_d2g[i][j] =
      _prop_d2g[j][i] = &declarePropertyDerivative<Real>(_function_name, eta_name[i], eta_name[j]);
    }
  }
}

void
OPInterfaceBarrierMaterial::computeQpProperties()
{
  // Initialize properties to zero before accumulating
  _prop_g[_qp] = 0.0;
  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    (*_prop_dg[i])[_qp] = 0.0;
    for (unsigned int j = i; j < _num_eta; ++j)
      (*_prop_d2g[i][j])[_qp] = 0.0;
  }

  // Create Interface barrier preventing interfaces involving more than two order parameters
  for (unsigned int i = 0; i < _num_eta; ++i)
    for (unsigned int j = 0; j < i; ++j)
      for (unsigned int k = 0; k < j; ++k)
      {
            _prop_g[_qp]         +=  (*_eta[i])[_qp] * (*_eta[i])[_qp] * (*_eta[j])[_qp] * (*_eta[j])[_qp] * (*_eta[k])[_qp] * (*_eta[k])[_qp];
            (*_prop_dg[i])[_qp]  +=  2 * (*_eta[i])[_qp] * (*_eta[j])[_qp] * (*_eta[j])[_qp] * (*_eta[k])[_qp] * (*_eta[k])[_qp];
            (*_prop_d2g[i][j])[_qp] +=  4 * (*_eta[i])[_qp] * (*_eta[j])[_qp] * (*_eta[k])[_qp] * (*_eta[k])[_qp];
      }
}
