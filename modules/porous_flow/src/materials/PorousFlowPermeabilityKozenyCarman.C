//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPermeabilityKozenyCarman.h"

registerMooseObject("PorousFlowApp", PorousFlowPermeabilityKozenyCarman);

InputParameters
PorousFlowPermeabilityKozenyCarman::validParams()
{
  InputParameters params = PorousFlowPermeabilityBase::validParams();
  MooseEnum poroperm_function("kozeny_carman_fd2=0 kozeny_carman_phi0=1", "kozeny_carman_fd2");
  params.addParam<MooseEnum>(
      "poroperm_function",
      poroperm_function,
      "Function relating porosity and permeability. The options are: kozeny_carman_fd2 = f d^2 "
      "phi^n/(1-phi)^m (where phi is porosity, f is a scalar constant with typical values "
      "0.01-0.001, and d is grain size). kozeny_carman_phi0 = k0 (1-phi0)^m/phi0^n * "
      "phi^n/(1-phi)^m (where phi is porosity, and k0 is the permeability at porosity phi0)");
  params.addRangeCheckedParam<Real>("k0",
                                    "k0 > 0",
                                    "The permeability scalar value (usually in "
                                    "m^2) at the reference porosity, required for "
                                    "kozeny_carman_phi0");
  params.addParam<RealTensorValue>("k_anisotropy",
                                   "A tensor to multiply the calculated scalar "
                                   "permeability, in order to obtain anisotropy if "
                                   "required. Defaults to isotropic permeability "
                                   "if not specified.");
  params.addRangeCheckedParam<Real>(
      "phi0", "phi0 > 0 & phi0 < 1", "The reference porosity, required for kozeny_carman_phi0");
  params.addRangeCheckedParam<Real>(
      "f", "f > 0", "The multiplying factor, required for kozeny_carman_fd2");
  params.addRangeCheckedParam<Real>(
      "d", "d > 0", "The grain diameter, required for kozeny_carman_fd2");
  params.addRequiredRangeCheckedParam<Real>("n", "n >= 0", "Porosity exponent");
  params.addRequiredRangeCheckedParam<Real>("m", "m >= 0", "(1-porosity) exponent");
  params.addClassDescription(
      "This Material calculates the permeability tensor from a form of the Kozeny-Carman equation, "
      "k = k_ijk * A * phi^n / (1 - phi)^m, where k_ijk is a tensor providing the anisotropy, phi "
      "is porosity, n and m are positive scalar constants and A is given in one of the following "
      "forms: A = k0 * (1 - phi0)^m / phi0^n (where k0 and phi0 are a reference permeability and "
      "porosity) or A = f * d^2 (where f is a scalar constant and d is grain diameter.");
  return params;
}

PorousFlowPermeabilityKozenyCarman::PorousFlowPermeabilityKozenyCarman(
    const InputParameters & parameters)
  : PorousFlowPermeabilityBase(parameters),
    _k0(parameters.isParamValid("k0") ? getParam<Real>("k0") : -1),
    _phi0(parameters.isParamValid("phi0") ? getParam<Real>("phi0") : -1),
    _f(parameters.isParamValid("f") ? getParam<Real>("f") : -1),
    _d(parameters.isParamValid("d") ? getParam<Real>("d") : -1),
    _m(getParam<Real>("m")),
    _n(getParam<Real>("n")),
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
    case PoropermFunction::kozeny_carman_fd2:
      if (!(parameters.isParamValid("f") && parameters.isParamValid("d")))
        mooseError("You must specify f and d in order to use kozeny_carman_fd2 in "
                   "PorousFlowPermeabilityKozenyCarman");
      _A = _f * _d * _d;
      break;

    case PoropermFunction::kozeny_carman_phi0:
      if (!(parameters.isParamValid("k0") && parameters.isParamValid("phi0")))
        mooseError("You must specify k0 and phi0 in order to use kozeny_carman_phi0 in "
                   "PorousFlowPermeabilityKozenyCarman");
      _A = _k0 * std::pow(1.0 - _phi0, _m) / std::pow(_phi0, _n);
      break;
  }

  // Make sure that derivatives are included in the Jacobian calculations
  _dictator.usePermDerivs(true);
}

void
PorousFlowPermeabilityKozenyCarman::computeQpProperties()
{
  _permeability_qp[_qp] =
      _k_anisotropy * _A * std::pow(_porosity_qp[_qp], _n) / std::pow(1.0 - _porosity_qp[_qp], _m);

  (*_dpermeability_qp_dvar)[_qp].resize(_num_var, RealTensorValue());
  for (unsigned int v = 0; v < _num_var; ++v)
    (*_dpermeability_qp_dvar)[_qp][v] = _dporosity_qp_dvar[_qp][v] * _permeability_qp[_qp] *
                                        (_n / _porosity_qp[_qp] + _m / (1.0 - _porosity_qp[_qp]));

  (*_dpermeability_qp_dgradvar)[_qp].resize(LIBMESH_DIM);
  for (unsigned i = 0; i < LIBMESH_DIM; ++i)
  {
    (*_dpermeability_qp_dgradvar)[_qp][i].resize(_num_var, RealTensorValue());
    for (unsigned int v = 0; v < _num_var; ++v)
      (*_dpermeability_qp_dgradvar)[_qp][i][v] =
          _dporosity_qp_dgradvar[_qp][v](i) * _permeability_qp[_qp] *
          (_n / _porosity_qp[_qp] + _m / (1.0 - _porosity_qp[_qp]));
  }
}
