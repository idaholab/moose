//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFluxCalculatorUnsaturated.h"

registerMooseObject("PorousFlowApp", PorousFlowAdvectiveFluxCalculatorUnsaturated);

InputParameters
PorousFlowAdvectiveFluxCalculatorUnsaturated::validParams()
{
  InputParameters params = PorousFlowAdvectiveFluxCalculatorSaturated::validParams();
  params.addClassDescription(
      "Computes the advective flux of fluid of given phase, assuming unsaturated conditions.  "
      "Hence this UserObject is only relevant to single-phase situations, or multi-phase "
      "situations where each fluid component appears in one phase only.  Explicitly, the "
      "UserObject computes (density * relative_permeability / viscosity) * (- permeability * "
      "(grad(P) - density * gravity)), using the Kuzmin-Turek FEM-TVD multidimensional "
      "stabilization scheme");
  return params;
}

PorousFlowAdvectiveFluxCalculatorUnsaturated::PorousFlowAdvectiveFluxCalculatorUnsaturated(
    const InputParameters & parameters)
  : PorousFlowAdvectiveFluxCalculatorSaturated(parameters),
    _relative_permeability(
        getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_nodal")),
    _drelative_permeability_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_relative_permeability_nodal_dvar"))
{
}

Real
PorousFlowAdvectiveFluxCalculatorUnsaturated::computeU(unsigned i) const
{
  return _relative_permeability[i][_phase] *
         PorousFlowAdvectiveFluxCalculatorSaturated::computeU(i);
}

Real
PorousFlowAdvectiveFluxCalculatorUnsaturated::computedU_dvar(unsigned i, unsigned pvar) const
{
  Real du = _drelative_permeability_dvar[i][_phase][pvar] *
            PorousFlowAdvectiveFluxCalculatorSaturated::computeU(i);
  du += _relative_permeability[i][_phase] *
        PorousFlowAdvectiveFluxCalculatorSaturated::computedU_dvar(i, pvar);
  return du;
}
