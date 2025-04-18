//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPermeabilityKozenyCarman.h"

registerMooseObject("PorousFlowApp", PorousFlowPermeabilityKozenyCarman);
registerMooseObject("PorousFlowApp", ADPorousFlowPermeabilityKozenyCarman);

template <bool is_ad>
InputParameters
PorousFlowPermeabilityKozenyCarmanTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowPermeabilityKozenyCarmanBase::validParams();
  MooseEnum poroperm_function("kozeny_carman_fd2=0 kozeny_carman_phi0=1 kozeny_carman_A=2",
                              "kozeny_carman_fd2");
  params.addParam<MooseEnum>(
      "poroperm_function",
      poroperm_function,
      "Function relating porosity and permeability. The options are: kozeny_carman_fd2 = f d^2 "
      "phi^n/(1-phi)^m (where phi is porosity, f is a scalar constant with typical values "
      "0.01-0.001, and d is grain size). kozeny_carman_phi0 = k0 (1-phi0)^m/phi0^n * "
      "phi^n/(1-phi)^m (where phi is porosity, and k0 is the permeability at porosity phi0)  "
      "kozeny_carman_A = A for directly supplying the permeability multiplying factor.");
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
  params.addRangeCheckedParam<Real>(
      "A", "A > 0", "Kozeny Carman permeability multiplying factor, required for kozeny_carman_A");
  params.addClassDescription("This Material calculates the permeability tensor from a form of the "
                             "Kozeny-Carman equation based on the spatially constant initial "
                             "permeability and porosity or grain size.");
  return params;
}

template <bool is_ad>
PorousFlowPermeabilityKozenyCarmanTempl<is_ad>::PorousFlowPermeabilityKozenyCarmanTempl(
    const InputParameters & parameters)
  : PorousFlowPermeabilityKozenyCarmanBaseTempl<is_ad>(parameters),
    _k0(parameters.isParamValid("k0") ? this->template getParam<Real>("k0") : -1),
    _phi0(parameters.isParamValid("phi0") ? this->template getParam<Real>("phi0") : -1),
    _f(parameters.isParamValid("f") ? this->template getParam<Real>("f") : -1),
    _d(parameters.isParamValid("d") ? this->template getParam<Real>("d") : -1),
    _poroperm_function(this->template getParam<MooseEnum>("poroperm_function")
                           .template getEnum<PoropermFunction>()),
    _A(parameters.isParamValid("A") ? this->template getParam<Real>("A") : -1)
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
      _A = _k0 * std::pow(1.0 - _phi0, this->_m) / std::pow(_phi0, this->_n);
      break;
    case PoropermFunction::kozeny_carman_A:
      if (!(parameters.isParamValid("A")))
        mooseError("You must specify A in order to use kozeny_carman_A in "
                   "PorousFlowPermeabilityKozenyCarman");
      break;
  }
}

template <bool is_ad>
Real
PorousFlowPermeabilityKozenyCarmanTempl<is_ad>::computeA() const
{
  return _A;
}

template class PorousFlowPermeabilityKozenyCarmanTempl<false>;
template class PorousFlowPermeabilityKozenyCarmanTempl<true>;
