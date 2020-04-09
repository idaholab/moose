//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent.h"

registerMooseObject("PorousFlowApp", PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent);

InputParameters
PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent::validParams()
{
  InputParameters params = PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent::validParams();
  params.addClassDescription(
      "Computes the advective flux of fluid of given phase and component.  Hence this UserObject "
      "is relevant to multi-phase, multi-component situations.  Explicitly, the UserObject "
      "computes (mass_fraction * density * relative_permeability / viscosity) * (- permeability * "
      "(grad(P) - density * gravity)), using the Kuzmin-Turek FEM-TVD multidimensional "
      "stabilization scheme");
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
PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent::computeU(unsigned i) const
{
  return _relative_permeability[i][_phase] *
         PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent::computeU(i);
}

Real
PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent::computedU_dvar(unsigned i,
                                                                           unsigned pvar) const
{
  Real du = _drelative_permeability_dvar[i][_phase][pvar] *
            PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent::computeU(i);
  du += _relative_permeability[i][_phase] *
        PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent::computedU_dvar(i, pvar);
  return du;
}
