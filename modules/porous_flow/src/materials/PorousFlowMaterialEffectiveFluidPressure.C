/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialEffectiveFluidPressure.h"

template<>
InputParameters validParams<PorousFlowMaterialEffectiveFluidPressure>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates an effective fluid pressure: effective_stress = total_stress + biot_coeff*effective_fluid_pressure.  The effective_fluid_pressure = sum_{phases}(S_phase * P_phase)");
  return params;
}

PorousFlowMaterialEffectiveFluidPressure::PorousFlowMaterialEffectiveFluidPressure(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
    _num_ph(_dictator_UO.num_phases()),
    _num_var(_dictator_UO.num_v()),

    _porepressure_qp(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_qp")),
    _dporepressure_qp_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_qp_dvar")),
    _saturation_qp(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_qp")),
    _dsaturation_qp_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_qp_dvar")),
    _pf(declareProperty<Real>("PorousFlow_effective_fluid_pressure")),
    _dpf_dvar(declareProperty<std::vector<Real> >("dPorousFlow_effective_fluid_pressure_dvar"))
{
}

void
PorousFlowMaterialEffectiveFluidPressure::computeQpProperties()
{
  _pf[_qp] = 0;
  _dpf_dvar[_qp].assign(_num_var, 0.0);
  for (unsigned ph = 0; ph < _num_ph; ++ph)
  {
    _pf[_qp] += _saturation_qp[_qp][ph]*_porepressure_qp[_qp][ph];
    for (unsigned v = 0; v < _num_var; ++v)
      _dpf_dvar[_qp][v] += _dsaturation_qp_dvar[_qp][ph][v]*_porepressure_qp[_qp][ph] + _saturation_qp[_qp][ph]*_dporepressure_qp_dvar[_qp][ph][v];
  }
}

