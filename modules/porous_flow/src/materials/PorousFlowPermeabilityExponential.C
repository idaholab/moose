//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPermeabilityExponential.h"

registerMooseObject("PorousFlowApp", PorousFlowPermeabilityExponential);

InputParameters
PorousFlowPermeabilityExponential::validParams()
{
  InputParameters params = PorousFlowPermeabilityBase::validParams();
  MooseEnum poroperm_function("log_k ln_k exp_k", "exp_k");
  params.addParam<MooseEnum>("poroperm_function",
                             poroperm_function,
                             "Form of the function relating porosity and permeability. The options "
                             "are: log_k (log k = A phi + B); ln_k (ln k = A phi + B); exp_k (k = "
                             "B exp(A phi)); where k is permeability, phi is porosity, A and B are "
                             "empirical constants.");
  params.addParam<RealTensorValue>("k_anisotropy",
                                   "A tensor to multiply the calculated scalar "
                                   "permeability, in order to obtain anisotropy if "
                                   "required. Defaults to isotropic permeability "
                                   "if not specified.");
  params.addRequiredParam<Real>("A", "Empirical constant; see poroperm_function.");
  params.addRequiredParam<Real>("B", "Empirical constant; see poroperm_function.");
  params.addClassDescription(
      "This Material calculates the permeability tensor from an exponential function of porosity: "
      "k = k_ijk * BB exp(AA phi), where k_ijk is a tensor providing the anisotropy, phi is "
      "porosity, and AA and BB are empirical constants. The user can provide input for the "
      "function expressed in ln k, log k or exponential forms (see poroperm_function).");
  return params;
}

PorousFlowPermeabilityExponential::PorousFlowPermeabilityExponential(
    const InputParameters & parameters)
  : PorousFlowPermeabilityBase(parameters),
    _A(getParam<Real>("A")),
    _B(getParam<Real>("B")),
    _k_anisotropy(parameters.isParamValid("k_anisotropy")
                      ? getParam<RealTensorValue>("k_anisotropy")
                      : RealTensorValue(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)),
    _porosity_qp(getMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _dporosity_qp_dvar(getMaterialProperty<std::vector<Real>>("dPorousFlow_porosity_qp_dvar")),
    _dporosity_qp_dgradvar(
        getMaterialProperty<std::vector<RealGradient>>("dPorousFlow_porosity_qp_dgradvar")),
    _poroperm_function(getParam<MooseEnum>("poroperm_function").getEnum<PoropermFunction>())
{
  switch (_poroperm_function)
  {
    case PoropermFunction::log_k:
      _AA = _A * std::log(10.0);
      _BB = std::pow(10.0, _B);
      break;

    case PoropermFunction::ln_k:
      _AA = _A;
      _BB = std::exp(_B);
      break;

    case PoropermFunction::exp_k:
      _AA = _A;
      _BB = _B;
      break;
  }

  // Make sure that derivatives are included in the Jacobian calculations
  _dictator.usePermDerivs(true);
}

void
PorousFlowPermeabilityExponential::computeQpProperties()
{
  _permeability_qp[_qp] = _k_anisotropy * _BB * std::exp(_porosity_qp[_qp] * _AA);

  (*_dpermeability_qp_dvar)[_qp].resize(_num_var, RealTensorValue());
  for (unsigned int v = 0; v < _num_var; ++v)
    (*_dpermeability_qp_dvar)[_qp][v] = _AA * _permeability_qp[_qp] * _dporosity_qp_dvar[_qp][v];

  (*_dpermeability_qp_dgradvar)[_qp].resize(LIBMESH_DIM);
  for (unsigned i = 0; i < LIBMESH_DIM; ++i)
  {
    (*_dpermeability_qp_dgradvar)[_qp][i].resize(_num_var, RealTensorValue());
    for (unsigned int v = 0; v < _num_var; ++v)
      (*_dpermeability_qp_dgradvar)[_qp][i][v] =
          _AA * _permeability_qp[_qp] * _dporosity_qp_dgradvar[_qp][v](i);
  }
}
