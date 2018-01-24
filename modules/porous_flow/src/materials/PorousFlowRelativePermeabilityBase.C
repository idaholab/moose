//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowRelativePermeabilityBase.h"

template <>
InputParameters
validParams<PorousFlowRelativePermeabilityBase>()
{
  InputParameters params = validParams<PorousFlowMaterialBase>();
  params.addRangeCheckedParam<Real>(
      "scaling", 1.0, "scaling>=0", "Relative permeability is multiplied by this factor");
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
  params.addClassDescription("Base class for PorousFlow relative permeability materials");
  return params;
}

PorousFlowRelativePermeabilityBase::PorousFlowRelativePermeabilityBase(
    const InputParameters & parameters)
  : PorousFlowMaterialBase(parameters),
    _scaling(getParam<Real>("scaling")),
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

  _relative_permeability[_qp] = relperm * _scaling;
  _drelative_permeability_ds[_qp] = drelperm * _dseff_ds * _scaling;
}

Real
PorousFlowRelativePermeabilityBase::effectiveSaturation(Real saturation) const
{
  return (saturation - _s_res) / (1.0 - _sum_s_res);
}
