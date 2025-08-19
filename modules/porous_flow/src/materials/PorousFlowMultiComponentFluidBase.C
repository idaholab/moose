//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMultiComponentFluidBase.h"

template <bool is_ad>
InputParameters
PorousFlowMultiComponentFluidBaseTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowFluidPropertiesBaseTempl<is_ad>::validParams();
  return params;
}

template <bool is_ad>
PorousFlowMultiComponentFluidBaseTempl<is_ad>::PorousFlowMultiComponentFluidBaseTempl(
    const InputParameters & parameters)
  : PorousFlowFluidPropertiesBaseTempl<is_ad>(parameters),
    _ddensity_dX((_compute_rho_mu && !is_ad)
                     ? (_nodal_material
                            ? &this->template declarePropertyDerivative<std::vector<Real>>(
                                  "PorousFlow_fluid_phase_density_nodal" + _phase,
                                  _mass_fraction_variable_name)
                            : &this->template declarePropertyDerivative<std::vector<Real>>(
                                  "PorousFlow_fluid_phase_density_qp" + _phase,
                                  _mass_fraction_variable_name))
                     : nullptr),
    _dviscosity_dX(
        (_compute_rho_mu && !is_ad)
            ? (_nodal_material
                   ? &this->template declarePropertyDerivative<std::vector<Real>>(
                         "PorousFlow_viscosity_nodal" + _phase, _mass_fraction_variable_name)
                   : &this->template declarePropertyDerivative<std::vector<Real>>(
                         "PorousFlow_viscosity_qp" + _phase, _mass_fraction_variable_name))
            : nullptr),
    _dinternal_energy_dX((_compute_internal_energy && !is_ad)
                             ? (_nodal_material
                                    ? &this->template declarePropertyDerivative<std::vector<Real>>(
                                          "PorousFlow_fluid_phase_internal_energy_nodal" + _phase,
                                          _mass_fraction_variable_name)
                                    : &this->template declarePropertyDerivative<std::vector<Real>>(
                                          "PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                          _mass_fraction_variable_name))
                             : nullptr),
    _denthalpy_dX((_compute_enthalpy && !is_ad)
                      ? (_nodal_material
                             ? &this->template declarePropertyDerivative<std::vector<Real>>(
                                   "PorousFlow_fluid_phase_enthalpy_nodal" + _phase,
                                   _mass_fraction_variable_name)
                             : &this->template declarePropertyDerivative<std::vector<Real>>(
                                   "PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                   _mass_fraction_variable_name))
                      : nullptr)
{
}

template <bool is_ad>
void
PorousFlowMultiComponentFluidBaseTempl<is_ad>::initQpStatefulProperties()
{
}

template <bool is_ad>
void
PorousFlowMultiComponentFluidBaseTempl<is_ad>::computeQpProperties()
{
}

template class PorousFlowMultiComponentFluidBaseTempl<false>;
template class PorousFlowMultiComponentFluidBaseTempl<true>;
