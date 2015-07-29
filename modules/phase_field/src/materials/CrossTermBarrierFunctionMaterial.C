/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrossTermBarrierFunctionMaterial.h"

template<>
InputParameters validParams<CrossTermBarrierFunctionMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("function_name", "g", "actual name for g(eta_i)");
  MooseEnum h_order("SIMPLE=0", "SIMPLE");
  params.addParam<MooseEnum>("g_order", h_order, "Polynomial order of the switching function h(eta)");
  params.addRequiredCoupledVar("etas", "eta_i order parameters, one for each h");
  params.addRequiredParam<std::vector<Real> >("W_ij", "Terms controlling barrier height set W=1 in DerivativeMultiPhaseMaterial for these to apply");
  return params;
}

CrossTermBarrierFunctionMaterial::CrossTermBarrierFunctionMaterial(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _function_name(getParam<std::string>("function_name")),
    _g_order(getParam<MooseEnum>("g_order")),
    _W_ij(getParam<std::vector<Real> >("W_ij")),
    _num_eta(coupledComponents("etas")),
    _eta(_num_eta),
    _prop_g(declareProperty<Real>(_function_name)),
    _prop_dg(_num_eta),
    _prop_d2g(_num_eta)

{
  // if Vector W_ij is not the correct size to fill the matrix give error
  if (_num_eta * _num_eta != _W_ij.size())
    mooseError("Supply the number of etas squared for W_ij.");

  // if W_ij is not symmetric or if diagonal values are not zero, return error
  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    if (_W_ij[_num_eta * i + i] != 0)
      mooseError("Set on-diagonal values of W_ij to zero");

    for (unsigned int j = i; j < _num_eta; ++j)
    {
      if (_W_ij[_num_eta * i + j] != _W_ij[_num_eta * j + i])
        mooseError("Supply symmetric values for W_ij");
    }
  }

  for (unsigned int i = 0; i < _num_eta; ++i)
    _prop_d2g[i].resize(_num_eta);

  // declare derivative properties, fetch eta values
  std::vector<std::string> eta_name(_num_eta);
  for (unsigned int i = 0; i < _num_eta; ++i)
    eta_name[i] = getVar("etas", i)->name();

  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    _prop_dg[i]  = &declarePropertyDerivative<Real>(_function_name, eta_name[i]);
    _eta[i] = &coupledValue("etas", i);
    for (unsigned int j = i; j < _num_eta; ++j)
    {
      _prop_d2g[i][j] =
      _prop_d2g[j][i] = &declarePropertyDerivative<Real>(_function_name, eta_name[i], eta_name[j]);
    }
  }
}

void
CrossTermBarrierFunctionMaterial::computeQpProperties()
{
  // Initialize properties to zero before accumulating
  _prop_g[_qp] = 0.0;
  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    (*_prop_dg[i])[_qp] = 0.0;
    for (unsigned int j = i; j < _num_eta; ++j)
      (*_prop_d2g[i][j])[_qp] = 0.0;
  }

  // Sum the components of our W_ij matrix to get constant used in our g function
  for (unsigned int i = 0; i < _num_eta; ++i)
    for (unsigned int j = i; j < _num_eta; ++j)
    {
      switch (_g_order)
      {
        case 0: // SIMPLE
          _prop_g[_qp]         +=  _W_ij[_num_eta * i + j] * (*_eta[i])[_qp] * (*_eta[i])[_qp] * (*_eta[j])[_qp] * (*_eta[j])[_qp];
          (*_prop_dg[i])[_qp]  +=  _W_ij[_num_eta * i + j] * 2 * (*_eta[i])[_qp] * (*_eta[j])[_qp] * (*_eta[j])[_qp];
          if (i == j)
            (*_prop_d2g[i][j])[_qp] +=  _W_ij[_num_eta * i + j] * 2 * (*_eta[j])[_qp] * (*_eta[j])[_qp];
          else
            (*_prop_d2g[i][j])[_qp] +=  _W_ij[_num_eta * i + j] * 4 * (*_eta[i])[_qp] * (*_eta[j])[_qp];
          break;
      }
    }
}
