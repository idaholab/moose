/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowRelativePermeabilityBase.h"

template <>
InputParameters
validParams<PorousFlowRelativePermeabilityBase>()
{
  InputParameters params = validParams<PorousFlowMaterialBase>();
  params.addRangeCheckedParam<Real>(
      "s_res",
      0,
      "s_res >= 0 & s_res < 1",
      "The residual saturation of the phase j. Must be between 0 and 1");
  params.addRangeCheckedParam<Real>(
      "sum_s_res",
      0,
      "sum_s_res >= 0 & sum_s_res < 1",
      "Sum of residual saturations over all phases.  Must be between 0 and 1");
  // Note for coders: currently only coded for nodal materials.  it is not difficult to generalise
  // to quadpoint materials!
  params.addClassDescription("Base class for PorousFlow relative permeability materials");
  return params;
}

PorousFlowRelativePermeabilityBase::PorousFlowRelativePermeabilityBase(
    const InputParameters & parameters)
  : PorousFlowMaterialBase(parameters),
    _saturation_variable_name(_dictator.saturationVariableNameDummy()),
    _saturation(_nodal_material
                    ? getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")
                    : getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp")),
    _relative_permeability(
        _nodal_material ? declareProperty<Real>("PorousFlow_relative_permeability_nodal" + _phase)
                        : declareProperty<Real>("PorousFlow_relative_permeability_qp" + _phase)),
    _drelative_permeability_ds(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_relative_permeability_nodal" + _phase,
                                              _saturation_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_relative_permeability_qp" + _phase,
                                              _saturation_variable_name)),
    _s_res(getParam<Real>("s_res")),
    _sum_s_res(getParam<Real>("sum_s_res")),
    _dseff_ds(1.0 / (1.0 - _sum_s_res))
{
  if (_sum_s_res < _s_res)
    mooseError("Sum of residual saturations sum_s_res cannot be smaller than s_res in ", name());
}

void
PorousFlowRelativePermeabilityBase::computeQpProperties()
{
  // Effective saturation
  Real seff = effectiveSaturation(_saturation[_qp][_phase_num]);
  Real relperm, drelperm;

  if (seff < 0.0)
  {
    // Relative permeability is 0 for saturation less than residual
    relperm = 0.0;
    drelperm = 0.0;
  }
  else if (seff >= 0.0 && seff <= 1)
  {
    relperm = relativePermeability(seff);
    drelperm = dRelativePermeability(seff);
  }
  else // seff > 1
  {
    // Relative permeability is 1 when fully saturated
    relperm = 1.0;
    drelperm = 0.0;
  }

  _relative_permeability[_qp] = relperm;
  _drelative_permeability_ds[_qp] = drelperm * _dseff_ds;
}

Real
PorousFlowRelativePermeabilityBase::effectiveSaturation(Real saturation) const
{
  return (saturation - _s_res) / (1.0 - _sum_s_res);
}
