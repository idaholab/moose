//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent.h"

registerMooseObject("PorousFlowApp", PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent);

template <>
InputParameters
validParams<PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent>()
{
  InputParameters params = validParams<PorousFlowAdvectiveFluxCalculatorSaturated>();
  params.addClassDescription(
      "TODO "
      "Compute K_ij (a measure of advective flux from node i to node j) "
      "and R+ and R- (which quantify amount of antidiffusion to add) in the "
      "Kuzmin-Turek FEM-TVD multidimensional scheme.  Constant advective velocity is assumed");
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index corresponding to the fluid component for this UserObject");
  return params;
}

PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent::
    PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent(const InputParameters & parameters)
  : PorousFlowAdvectiveFluxCalculatorSaturated(parameters),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _mass_fractions(
        getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")),
    _dmass_fractions_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
        "dPorousFlow_mass_frac_nodal_dvar"))
{
  if (_fluid_component >= _dictator.numComponents())
    paramError("fluid_component",
               "Fluid component number entered is greater than the number of fluid components "
               "specified in the Dictator. Remember that indexing starts at 0");
}

Real
PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent::getU(unsigned i) const
{
  return _mass_fractions[i][_phase][_fluid_component] *
         PorousFlowAdvectiveFluxCalculatorSaturated::getU(i);
}

Real
PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent::dU_dvar(unsigned i, unsigned pvar) const
{
  Real du = _mass_fractions[i][_phase][_fluid_component] *
            PorousFlowAdvectiveFluxCalculatorSaturated::dU_dvar(i, pvar);
  du += _dmass_fractions_dvar[i][_phase][_fluid_component][pvar] *
        PorousFlowAdvectiveFluxCalculatorSaturated::getU(i);
  return du;
}
