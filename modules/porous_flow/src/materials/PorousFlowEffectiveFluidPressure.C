//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowEffectiveFluidPressure.h"

registerMooseObject("PorousFlowApp", PorousFlowEffectiveFluidPressure);
registerMooseObject("PorousFlowApp", ADPorousFlowEffectiveFluidPressure);

template <bool is_ad>
InputParameters
PorousFlowEffectiveFluidPressureTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.set<std::string>("pf_material_type") = "effective_pressure";
  params.addClassDescription("This Material calculates an effective fluid pressure: "
                             "effective_stress = total_stress + "
                             "biot_coeff*effective_fluid_pressure.  The effective_fluid_pressure = "
                             "sum_{phases}(S_phase * P_phase)");
  return params;
}

template <bool is_ad>
PorousFlowEffectiveFluidPressureTempl<is_ad>::PorousFlowEffectiveFluidPressureTempl(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _porepressure(
        _nodal_material
            ? getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_porepressure_nodal")
            : getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_porepressure_qp")),
    _dporepressure_dvar(is_ad             ? nullptr
                        : _nodal_material ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                                "dPorousFlow_porepressure_nodal_dvar")
                                          : &getMaterialProperty<std::vector<std::vector<Real>>>(
                                                "dPorousFlow_porepressure_qp_dvar")),
    _saturation(
        _nodal_material
            ? getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_saturation_nodal")
            : getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_saturation_qp")),
    _dsaturation_dvar(is_ad             ? nullptr
                      : _nodal_material ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                              "dPorousFlow_saturation_nodal_dvar")
                                        : &getMaterialProperty<std::vector<std::vector<Real>>>(
                                              "dPorousFlow_saturation_qp_dvar")),
    _pf(_nodal_material
            ? declareGenericProperty<Real, is_ad>("PorousFlow_effective_fluid_pressure_nodal")
            : declareGenericProperty<Real, is_ad>("PorousFlow_effective_fluid_pressure_qp")),
    _dpf_dvar(
        is_ad ? nullptr
        : _nodal_material
            ? &declareProperty<std::vector<Real>>("dPorousFlow_effective_fluid_pressure_nodal_dvar")
            : &declareProperty<std::vector<Real>>("dPorousFlow_effective_fluid_pressure_qp_dvar"))
{
}

template <bool is_ad>
void
PorousFlowEffectiveFluidPressureTempl<is_ad>::initQpStatefulProperties()
{
  _pf[_qp] = 0.0;
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _pf[_qp] += _saturation[_qp][ph] * _porepressure[_qp][ph];
}

template <bool is_ad>
void
PorousFlowEffectiveFluidPressureTempl<is_ad>::computeQpProperties()
{
  _pf[_qp] = 0.0;

  if (!is_ad)
    (*_dpf_dvar)[_qp].assign(_num_var, 0.0);

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _pf[_qp] += _saturation[_qp][ph] * _porepressure[_qp][ph];

    if constexpr (!is_ad)
      for (unsigned int v = 0; v < _num_var; ++v)
        (*_dpf_dvar)[_qp][v] += (*_dsaturation_dvar)[_qp][ph][v] * _porepressure[_qp][ph] +
                                _saturation[_qp][ph] * (*_dporepressure_dvar)[_qp][ph][v];
  }
}
