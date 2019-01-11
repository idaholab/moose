//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFluxCalculatorUnsaturated.h"

registerMooseObject("PorousFlowApp", PorousFlowAdvectiveFluxCalculatorUnsaturated);

template <>
InputParameters
validParams<PorousFlowAdvectiveFluxCalculatorUnsaturated>()
{
  InputParameters params = validParams<PorousFlowAdvectiveFluxCalculatorSaturated>();
  params.addClassDescription(
      "TODO "
      "Compute K_ij (a measure of advective flux from node i to node j) "
      "and R+ and R- (which quantify amount of antidiffusion to add) in the "
      "Kuzmin-Turek FEM-TVD multidimensional scheme.  Constant advective velocity is assumed");
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
PorousFlowAdvectiveFluxCalculatorUnsaturated::getU(unsigned i) const
{
  return _relative_permeability[i][_phase] * PorousFlowAdvectiveFluxCalculatorSaturated::getU(i);
}

Real
PorousFlowAdvectiveFluxCalculatorUnsaturated::dU_dvar(unsigned i, unsigned pvar) const
{
  Real du = _drelative_permeability_dvar[i][_phase][pvar] *
            PorousFlowAdvectiveFluxCalculatorSaturated::getU(i);
  du += _relative_permeability[i][_phase] *
        PorousFlowAdvectiveFluxCalculatorSaturated::dU_dvar(i, pvar);
  return du;
}
