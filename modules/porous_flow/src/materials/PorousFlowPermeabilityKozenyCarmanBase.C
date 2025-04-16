//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPermeabilityKozenyCarmanBase.h"

template <bool is_ad>
InputParameters
PorousFlowPermeabilityKozenyCarmanBaseTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowPermeabilityBase::validParams();
  params.addParam<RealTensorValue>("k_anisotropy",
                                   "A tensor to multiply the calculated scalar "
                                   "permeability, in order to obtain anisotropy if "
                                   "required. Defaults to isotropic permeability "
                                   "if not specified.");
  params.addRequiredRangeCheckedParam<Real>("n", "n >= 0", "Porosity exponent");
  params.addRequiredRangeCheckedParam<Real>("m", "m >= 0", "(1-porosity) exponent");
  params.addClassDescription(
      "Base class for material that calculates the permeability tensor from a form of the Kozeny-Carman equation, "
      "k = k_ijk * A * phi^n / (1 - phi)^m, where k_ijk is a tensor providing the anisotropy, phi "
      "is porosity, n and m are positive scalar constants.  Method for computing A is given in the derived classes.");
  return params;
}

template <bool is_ad>
PorousFlowPermeabilityKozenyCarmanBaseTempl<is_ad>::PorousFlowPermeabilityKozenyCarmanBaseTempl(
    const InputParameters & parameters)
  : PorousFlowPermeabilityBaseTempl<is_ad>(parameters),
    _m(this->template getParam<Real>("m")),
    _n(this->template getParam<Real>("n")),
    _k_anisotropy(parameters.isParamValid("k_anisotropy")
                      ? this->template getParam<RealTensorValue>("k_anisotropy")
                      : RealTensorValue(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)),
    _porosity_qp(this->template getGenericMaterialProperty<Real, is_ad>("PorousFlow_porosity_qp")),
    _dporosity_qp_dvar(is_ad ? nullptr
                             : &this->template getMaterialProperty<std::vector<Real>>(
                                   "dPorousFlow_porosity_qp_dvar")),
    _dporosity_qp_dgradvar(is_ad ? nullptr
                                 : &this->template getMaterialProperty<std::vector<RealGradient>>(
                                       "dPorousFlow_porosity_qp_dgradvar"))
{
  // Make sure that derivatives are included in the Jacobian calculations
  _dictator.usePermDerivs(true);
}

template <bool is_ad>
void
PorousFlowPermeabilityKozenyCarmanBaseTempl<is_ad>::computeQpProperties()
{
  Real A = computeA();
  _permeability_qp[_qp] =
      _k_anisotropy * A * std::pow(_porosity_qp[_qp], _n) / std::pow(1.0 - _porosity_qp[_qp], _m);

  if constexpr (!is_ad)
  {
    (*_dpermeability_qp_dvar)[_qp].resize(_num_var, RealTensorValue());
    for (unsigned int v = 0; v < _num_var; ++v)
      (*_dpermeability_qp_dvar)[_qp][v] = (*_dporosity_qp_dvar)[_qp][v] * _permeability_qp[_qp] *
                                          (_n / _porosity_qp[_qp] + _m / (1.0 - _porosity_qp[_qp]));

    (*_dpermeability_qp_dgradvar)[_qp].resize(LIBMESH_DIM);
    for (const auto i : make_range(Moose::dim))
    {
      (*_dpermeability_qp_dgradvar)[_qp][i].resize(_num_var, RealTensorValue());
      for (unsigned int v = 0; v < _num_var; ++v)
        (*_dpermeability_qp_dgradvar)[_qp][i][v] =
            (*_dporosity_qp_dgradvar)[_qp][v](i) * _permeability_qp[_qp] *
            (_n / _porosity_qp[_qp] + _m / (1.0 - _porosity_qp[_qp]));
    }
  }
}

template class PorousFlowPermeabilityKozenyCarmanBaseTempl<false>;
template class PorousFlowPermeabilityKozenyCarmanBaseTempl<true>;
