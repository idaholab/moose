//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowThermalConductivityFromPorosity.h"

registerMooseObject("PorousFlowApp", PorousFlowThermalConductivityFromPorosity);
registerMooseObject("PorousFlowApp", ADPorousFlowThermalConductivityFromPorosity);

template <bool is_ad>
InputParameters
PorousFlowThermalConductivityFromPorosityTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowThermalConductivityBaseTempl<is_ad>::validParams();
  params.addRequiredParam<RealTensorValue>("lambda_s",
                                           "The thermal conductivity of the solid matrix material");
  params.addRequiredParam<RealTensorValue>("lambda_f",
                                           "The thermal conductivity of the single fluid phase");
  params.addClassDescription("This Material calculates rock-fluid combined thermal conductivity "
                             "for the single phase, fully saturated case by using a linear "
                             "weighted average. "
                             "Thermal conductivity = phi * lambda_f + (1 - phi) * lambda_s, "
                             "where phi is porosity, and lambda_f, lambda_s are "
                             "thermal conductivities of the fluid and solid (assumed constant)");
  return params;
}

template <bool is_ad>
PorousFlowThermalConductivityFromPorosityTempl<
    is_ad>::PorousFlowThermalConductivityFromPorosityTempl(const InputParameters & parameters)
  : PorousFlowThermalConductivityBaseTempl<is_ad>(parameters),
    _la_s(this->template getParam<RealTensorValue>("lambda_s")),
    _la_f(this->template getParam<RealTensorValue>("lambda_f")),
    _porosity_qp(this->template getGenericMaterialProperty<Real, is_ad>("PorousFlow_porosity_qp")),
    _dporosity_qp_dvar(is_ad ? nullptr
                             : &this->template getMaterialProperty<std::vector<Real>>(
                                   "dPorousFlow_porosity_qp_dvar"))
{
  if (_num_phases != 1)
    this->template paramError("fluid_phase",
                              "The Dictator proclaims that the number of phases is ",
                              _dictator.numPhases(),
                              " whereas this material can only be used for single phase "
                              "simulations.  Be aware that the Dictator has noted your mistake.");
}

template <bool is_ad>
void
PorousFlowThermalConductivityFromPorosityTempl<is_ad>::computeQpProperties()
{
  _la_qp[_qp] = _la_s * (1.0 - _porosity_qp[_qp]) + _la_f * _porosity_qp[_qp];

  if constexpr (!is_ad)
  {
    (*_dla_qp_dvar)[_qp].assign(_num_var, RealTensorValue());
    for (const auto v : make_range(_num_var))
      (*_dla_qp_dvar)[_qp][v] = (_la_f - _la_s) * (*_dporosity_qp_dvar)[_qp][v];
  }
}

template class PorousFlowThermalConductivityFromPorosityTempl<false>;
template class PorousFlowThermalConductivityFromPorosityTempl<true>;
