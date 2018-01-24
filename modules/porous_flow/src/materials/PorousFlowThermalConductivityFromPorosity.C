//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowThermalConductivityFromPorosity.h"

template <>
InputParameters
validParams<PorousFlowThermalConductivityFromPorosity>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addRequiredParam<RealTensorValue>("lambda_s",
                                           "The thermal conductivity of the solid matrix material");
  params.addRequiredParam<RealTensorValue>("lambda_f",
                                           "The thermal conductivity of the single fluid phase");
  params.set<bool>("at_nodes") = false;
  params.addClassDescription("This Material calculates rock-fluid combined thermal conductivity "
                             "for the single phase, fully saturated case by using a linear "
                             "weighted average. "
                             "Thermal conductivity = phi * lambda_f + (1 - phi) * lambda_s, "
                             "where phi is porosity, and lambda_f, lambda_s are "
                             "thermal conductivities of the fluid and solid (assumed constant)");
  return params;
}

PorousFlowThermalConductivityFromPorosity::PorousFlowThermalConductivityFromPorosity(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _la_s(getParam<RealTensorValue>("lambda_s")),
    _la_f(getParam<RealTensorValue>("lambda_f")),
    _porosity_qp(getMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _dporosity_qp_dvar(getMaterialProperty<std::vector<Real>>("dPorousFlow_porosity_qp_dvar")),
    _la_qp(declareProperty<RealTensorValue>("PorousFlow_thermal_conductivity_qp")),
    _dla_qp_dvar(
        declareProperty<std::vector<RealTensorValue>>("dPorousFlow_thermal_conductivity_qp_dvar"))
{
  if (_num_phases != 1)
    mooseError("The Dictator proclaims that the number of phases is ",
               _dictator.numPhases(),
               " whereas PorousFlowThermalConductivityFromPorosity can only be used for 1-phase "
               "simulations.  Be aware "
               "that the Dictator has noted your mistake.");
  if (_nodal_material == true)
    mooseError("PorousFlowThermalConductivity classes are only defined for at_nodes = false");
}

void
PorousFlowThermalConductivityFromPorosity::computeQpProperties()
{
  _la_qp[_qp] = _la_s * (1.0 - _porosity_qp[_qp]) + _la_f * _porosity_qp[_qp];

  _dla_qp_dvar[_qp].assign(_num_var, RealTensorValue());
  for (unsigned v = 0; v < _num_var; ++v)
    _dla_qp_dvar[_qp][v] = (_la_f - _la_s) * _dporosity_qp_dvar[_qp][v];
}
