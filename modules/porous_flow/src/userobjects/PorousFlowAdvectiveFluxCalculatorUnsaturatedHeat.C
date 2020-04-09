//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat.h"

registerMooseObject("PorousFlowApp", PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat);

InputParameters
PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat::validParams()
{
  InputParameters params = PorousFlowAdvectiveFluxCalculatorSaturatedHeat::validParams();
  params.addClassDescription(
      "Computes the advective flux of heat energy in a given phase, assuming unsaturated "
      "conditions.  Hence this UserObject is only relevant to single-phase situations, or "
      "multi-phase situations where each fluid component appears in one phase only.  Explicitly, "
      "the UserObject computes (density * enthalpy * relative_permeability / viscosity) * (- "
      "permeability * (grad(P) - density * gravity)), using the Kuzmin-Turek FEM-TVD "
      "multidimensional stabilization scheme");
  return params;
}

PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat::PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat(
    const InputParameters & parameters)
  : PorousFlowAdvectiveFluxCalculatorSaturatedHeat(parameters),
    _relative_permeability(
        getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_nodal")),
    _drelative_permeability_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_relative_permeability_nodal_dvar"))
{
}

Real
PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat::computeU(unsigned i) const
{
  return _relative_permeability[i][_phase] *
         PorousFlowAdvectiveFluxCalculatorSaturatedHeat::computeU(i);
}

Real
PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat::computedU_dvar(unsigned i, unsigned pvar) const
{
  Real du = _drelative_permeability_dvar[i][_phase][pvar] *
            PorousFlowAdvectiveFluxCalculatorSaturatedHeat::computeU(i);
  du += _relative_permeability[i][_phase] *
        PorousFlowAdvectiveFluxCalculatorSaturatedHeat::computedU_dvar(i, pvar);
  return du;
}
