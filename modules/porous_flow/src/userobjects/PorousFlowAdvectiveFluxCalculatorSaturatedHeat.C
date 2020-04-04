//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFluxCalculatorSaturatedHeat.h"

registerMooseObject("PorousFlowApp", PorousFlowAdvectiveFluxCalculatorSaturatedHeat);

InputParameters
PorousFlowAdvectiveFluxCalculatorSaturatedHeat::validParams()
{
  InputParameters params = PorousFlowAdvectiveFluxCalculatorSaturated::validParams();
  params.addClassDescription(
      "Computes the advective flux of heat energy in the given phase, assuming fully-saturated "
      "conditions.  Hence this UserObject is only relevant to single-phase situations.  "
      "Explicitly, the UserObject computes (density * enthalpy / viscosity) * (- permeability * "
      "(grad(P) - density * gravity)), using the Kuzmin-Turek FEM-TVD multidimensional "
      "stabilization scheme");
  return params;
}

PorousFlowAdvectiveFluxCalculatorSaturatedHeat::PorousFlowAdvectiveFluxCalculatorSaturatedHeat(
    const InputParameters & parameters)
  : PorousFlowAdvectiveFluxCalculatorSaturated(parameters),
    _enthalpy(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_nodal")),
    _denthalpy_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_enthalpy_nodal_dvar"))
{
}

Real
PorousFlowAdvectiveFluxCalculatorSaturatedHeat::computeU(unsigned i) const
{
  return _enthalpy[i][_phase] * PorousFlowAdvectiveFluxCalculatorSaturated::computeU(i);
}

Real
PorousFlowAdvectiveFluxCalculatorSaturatedHeat::computedU_dvar(unsigned i, unsigned pvar) const
{
  Real du =
      _denthalpy_dvar[i][_phase][pvar] * PorousFlowAdvectiveFluxCalculatorSaturated::computeU(i);
  du += _enthalpy[i][_phase] * PorousFlowAdvectiveFluxCalculatorSaturated::computedU_dvar(i, pvar);
  return du;
}
