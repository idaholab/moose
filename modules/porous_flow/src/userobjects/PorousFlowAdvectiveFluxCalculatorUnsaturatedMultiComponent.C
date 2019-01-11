//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent.h"

registerMooseObject("PorousFlowApp", PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent);

template <>
InputParameters
validParams<PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent>()
{
  InputParameters params = validParams<PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent>();
  params.addClassDescription(
      "TODO "
      "Compute K_ij (a measure of advective flux from node i to node j) "
      "and R+ and R- (which quantify amount of antidiffusion to add) in the "
      "Kuzmin-Turek FEM-TVD multidimensional scheme.  Constant advective velocity is assumed");
  return params;
}

PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent::
    PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent(const InputParameters & parameters)
  : PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent(parameters),
    _relative_permeability(
        getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_nodal")),
    _drelative_permeability_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_relative_permeability_nodal_dvar"))
{
}

Real
PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent::getU(unsigned i) const
{
  return _relative_permeability[i][_phase] *
         PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent::getU(i);
}

Real
PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent::dU_dvar(unsigned i, unsigned pvar) const
{
  Real du = _drelative_permeability_dvar[i][_phase][pvar] *
            PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent::getU(i);
  du += _relative_permeability[i][_phase] *
        PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent::dU_dvar(i, pvar);
  return du;
}
