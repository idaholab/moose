/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowThermalConductivityIdeal.h"

template <>
InputParameters
validParams<PorousFlowThermalConductivityIdeal>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addRequiredParam<RealTensorValue>(
      "dry_thermal_conductivity",
      "The thermal conductivity of the rock matrix when the aqueous saturation is zero");
  params.addParam<RealTensorValue>("wet_thermal_conductivity",
                                   "The thermal conductivity of the rock matrix when the aqueous "
                                   "saturation is unity.  This defaults to "
                                   "dry_thermal_conductivity.");
  params.addParam<Real>("exponent",
                        1.0,
                        "Exponent on saturation.  Thermal conductivity = "
                        "dry_thermal_conductivity + S^exponent * "
                        "(wet_thermal_conductivity - dry_thermal_conductivity), "
                        "where S is the aqueous saturation");
  params.addParam<unsigned>("aqueous_phase_number",
                            0,
                            "The phase number of the aqueous phase.  In simulations without "
                            "fluids, this parameter and the exponent parameter will not be "
                            "used: only the dry_thermal_conductivity will be used.");
  params.set<bool>("at_nodes") = false;
  params.addClassDescription("This Material calculates rock-fluid combined thermal conductivity by "
                             "using a weighted sum.  Thermal conductivity = "
                             "dry_thermal_conductivity + S^exponent * (wet_thermal_conductivity - "
                             "dry_thermal_conductivity), where S is the aqueous saturation");
  return params;
}

PorousFlowThermalConductivityIdeal::PorousFlowThermalConductivityIdeal(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _la_dry(getParam<RealTensorValue>("dry_thermal_conductivity")),
    _wet_and_dry_differ(parameters.isParamValid("wet_thermal_conductivity")),
    _la_wet(_wet_and_dry_differ ? getParam<RealTensorValue>("wet_thermal_conductivity")
                                : getParam<RealTensorValue>("dry_thermal_conductivity")),
    _exponent(getParam<Real>("exponent")),
    _aqueous_phase(_num_phases > 0),
    _aqueous_phase_number(getParam<unsigned>("aqueous_phase_number")),
    _saturation_qp(_aqueous_phase
                       ? &getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp")
                       : nullptr),
    _dsaturation_qp_dvar(
        _aqueous_phase
            ? &getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_saturation_qp_dvar")
            : nullptr),
    _la_qp(declareProperty<RealTensorValue>("PorousFlow_thermal_conductivity_qp")),
    _dla_qp_dvar(
        declareProperty<std::vector<RealTensorValue>>("dPorousFlow_thermal_conductivity_qp_dvar"))
{
  if (_aqueous_phase && (_aqueous_phase_number >= _num_phases))
    mooseError("PorousFlowThermalConductivityIdeal: Your aqueous phase number, ",
               _aqueous_phase_number,
               " must not exceed the number of fluid phases in the system, which is ",
               _num_phases,
               "\n");
  if (_nodal_material == true)
    mooseError("PorousFlowThermalConductivity classes are only defined for at_nodes = false");
}

void
PorousFlowThermalConductivityIdeal::computeQpProperties()
{
  _la_qp[_qp] = _la_dry;
  if (_aqueous_phase && _wet_and_dry_differ)
    _la_qp[_qp] +=
        std::pow((*_saturation_qp)[_qp][_aqueous_phase_number], _exponent) * (_la_wet - _la_dry);

  _dla_qp_dvar[_qp].assign(_num_var, RealTensorValue());
  if (_aqueous_phase && _wet_and_dry_differ)
    for (unsigned v = 0; v < _num_var; ++v)
      _dla_qp_dvar[_qp][v] =
          _exponent * std::pow((*_saturation_qp)[_qp][_aqueous_phase_number], _exponent - 1.0) *
          (*_dsaturation_qp_dvar)[_qp][_aqueous_phase_number][v] * (_la_wet - _la_dry);
}
