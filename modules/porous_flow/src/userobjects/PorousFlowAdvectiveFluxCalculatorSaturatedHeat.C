//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFluxCalculatorSaturatedHeat.h"

registerMooseObject("PorousFlowApp", PorousFlowAdvectiveFluxCalculatorSaturatedHeat);

template <>
InputParameters
validParams<PorousFlowAdvectiveFluxCalculatorSaturatedHeat>()
{
  InputParameters params = validParams<PorousFlowAdvectiveFluxCalculatorSaturated>();
  params.addClassDescription(
      "TODO "
      "Compute K_ij (a measure of advective flux from node i to node j) "
      "and R+ and R- (which quantify amount of antidiffusion to add) in the "
      "Kuzmin-Turek FEM-TVD multidimensional scheme.  Constant advective velocity is assumed");
  return params;
}

PorousFlowAdvectiveFluxCalculatorSaturatedHeat::PorousFlowAdvectiveFluxCalculatorSaturatedHeat(
    const InputParameters & parameters)
  : PorousFlowAdvectiveFluxCalculatorSaturated(parameters),
    _enthalpy(getMaterialProperty<std::vector<Real>>("PorousFlow_enthalpy_nodal")),
    _denthalpy_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_enthalpy_nodal_dvar"))
{
}

Real
PorousFlowAdvectiveFluxCalculatorSaturatedHeat::getU(unsigned i) const
{
  return _enthalpy[i][_phase] * PorousFlowAdvectiveFluxCalculatorSaturated::getU(i);
}

Real
PorousFlowAdvectiveFluxCalculatorSaturatedHeat::dU_dvar(unsigned i, unsigned pvar) const
{
  Real du = _denthalpy_dvar[i][_phase][pvar] * PorousFlowAdvectiveFluxCalculatorSaturated::getU(i);
  du += _enthalpy[i][_phase] * PorousFlowAdvectiveFluxCalculatorSaturated::dU_dvar(i, pvar);
  return du;
}
