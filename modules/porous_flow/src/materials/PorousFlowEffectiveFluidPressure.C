/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowEffectiveFluidPressure.h"

template <>
InputParameters
validParams<PorousFlowEffectiveFluidPressure>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addClassDescription("This Material calculates an effective fluid pressure: "
                             "effective_stress = total_stress + "
                             "biot_coeff*effective_fluid_pressure.  The effective_fluid_pressure = "
                             "sum_{phases}(S_phase * P_phase)");
  return params;
}

PorousFlowEffectiveFluidPressure::PorousFlowEffectiveFluidPressure(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _porepressure(_nodal_material
                      ? getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_nodal")
                      : getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _porepressure_old(
        _nodal_material ? getMaterialPropertyOld<std::vector<Real>>("PorousFlow_porepressure_nodal")
                        : getMaterialPropertyOld<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _dporepressure_dvar(_nodal_material ? getMaterialProperty<std::vector<std::vector<Real>>>(
                                              "dPorousFlow_porepressure_nodal_dvar")
                                        : getMaterialProperty<std::vector<std::vector<Real>>>(
                                              "dPorousFlow_porepressure_qp_dvar")),
    _saturation(_nodal_material
                    ? getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")
                    : getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp")),
    _saturation_old(_nodal_material
                        ? getMaterialPropertyOld<std::vector<Real>>("PorousFlow_saturation_nodal")
                        : getMaterialPropertyOld<std::vector<Real>>("PorousFlow_saturation_qp")),
    _dsaturation_dvar(_nodal_material ? getMaterialProperty<std::vector<std::vector<Real>>>(
                                            "dPorousFlow_saturation_nodal_dvar")
                                      : getMaterialProperty<std::vector<std::vector<Real>>>(
                                            "dPorousFlow_saturation_qp_dvar")),
    _pf(_nodal_material ? declareProperty<Real>("PorousFlow_effective_fluid_pressure_nodal")
                        : declareProperty<Real>("PorousFlow_effective_fluid_pressure_qp")),
    _dpf_dvar(
        _nodal_material
            ? declareProperty<std::vector<Real>>("dPorousFlow_effective_fluid_pressure_nodal_dvar")
            : declareProperty<std::vector<Real>>("dPorousFlow_effective_fluid_pressure_qp_dvar"))
{
}

void
PorousFlowEffectiveFluidPressure::initQpStatefulProperties()
{
  _pf[_qp] = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
    _pf[_qp] += _saturation[_qp][ph] * _porepressure[_qp][ph];
}

void
PorousFlowEffectiveFluidPressure::computeQpProperties()
{
  _pf[_qp] = 0.0;
  _dpf_dvar[_qp].assign(_num_var, 0.0);
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    _pf[_qp] += _saturation[_qp][ph] * _porepressure[_qp][ph];
    for (unsigned v = 0; v < _num_var; ++v)
      _dpf_dvar[_qp][v] += _dsaturation_dvar[_qp][ph][v] * _porepressure[_qp][ph] +
                           _saturation[_qp][ph] * _dporepressure_dvar[_qp][ph][v];
  }
}
