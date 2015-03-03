/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MultiBarrierFunctionMaterial.h"

template<>
InputParameters validParams<MultiBarrierFunctionMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("function_name", "g", "actual name for g(eta_i)");
  MooseEnum h_order("SIMPLE=0", "SIMPLE");
  params.addParam<MooseEnum>("g_order", h_order, "Polynomial order of the switching function h(eta)");
  params.addRequiredCoupledVar("etas", "eta_i order parameters, one for each h");
  return params;
}

MultiBarrierFunctionMaterial::MultiBarrierFunctionMaterial(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<Material>(name, parameters),
    _function_name(getParam<std::string>("function_name")),
    _g_order(getParam<MooseEnum>("g_order")),
    _num_eta(coupledComponents("etas")),
    _eta(_num_eta),
    _prop_g(declareProperty<Real>(_function_name)),
    _prop_dg(_num_eta),
    _prop_d2g(_num_eta)
{
  // declare derivative properties, fetch eta values
  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    const std::string & eta_name = getVar("etas", i)->name();
    _prop_dg[i]  = &declarePropertyDerivative<Real>(_function_name, eta_name);
    _prop_d2g[i] = &declarePropertyDerivative<Real>(_function_name, eta_name, eta_name);
    _eta[i] = &coupledValue("etas", i);
  }
}

void
MultiBarrierFunctionMaterial::computeQpProperties()
{
  Real g = 0.0;

  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    const Real n = (*_eta[i])[_qp];
    switch (_g_order)
    {
      case 0: // SIMPLE
        g                   +=  n*n * (1.0 - n) * (1.0 - n);
        (*_prop_dg[i])[_qp]  =  2.0 * n * (n - 1.0) * (2.0 * n - 1.0);
        (*_prop_d2g[i])[_qp] = 12.0 * (n*n - n) + 2.0;
        break;
    }
  }

  _prop_g[_qp] = g;
}
