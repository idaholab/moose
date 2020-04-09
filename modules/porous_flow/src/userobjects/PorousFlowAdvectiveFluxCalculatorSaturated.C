//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFluxCalculatorSaturated.h"

registerMooseObject("PorousFlowApp", PorousFlowAdvectiveFluxCalculatorSaturated);

InputParameters
PorousFlowAdvectiveFluxCalculatorSaturated::validParams()
{
  InputParameters params = PorousFlowAdvectiveFluxCalculatorBase::validParams();
  params.addClassDescription(
      "Computes the advective flux of fluid of given phase, assuming fully-saturated conditions.  "
      "Hence this UserObject is only relevant to single-phase situations.  Explicitly, the "
      "UserObject computes (density / viscosity) * (- permeability * (grad(P) - density * "
      "gravity)), using the Kuzmin-Turek FEM-TVD multidimensional stabilization scheme");
  params.addParam<bool>(
      "multiply_by_density",
      true,
      "If true, then the advective flux will be multiplied by density, so it is a mass flux, which "
      "is the most common way of using PorousFlow.  If false, then the advective flux will be a "
      "volume flux, which is common in poro-mechanics");
  return params;
}

PorousFlowAdvectiveFluxCalculatorSaturated::PorousFlowAdvectiveFluxCalculatorSaturated(
    const InputParameters & parameters)
  : PorousFlowAdvectiveFluxCalculatorBase(parameters),
    _multiply_by_density(getParam<bool>("multiply_by_density")),
    _fluid_density_node(_multiply_by_density ? &getMaterialProperty<std::vector<Real>>(
                                                   "PorousFlow_fluid_phase_density_nodal")
                                             : nullptr),
    _dfluid_density_node_dvar(_multiply_by_density
                                  ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                        "dPorousFlow_fluid_phase_density_nodal_dvar")
                                  : nullptr),
    _fluid_viscosity(getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_nodal")),
    _dfluid_viscosity_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_nodal_dvar"))
{
}

Real
PorousFlowAdvectiveFluxCalculatorSaturated::computeU(unsigned i) const
{
  // The following is but one choice.
  // If you change this, you will probably have to change
  // - computeVelocity
  // - the derivative in executeOnElement
  // - computedU_dvar
  if (_multiply_by_density)
    return (*_fluid_density_node)[i][_phase] / _fluid_viscosity[i][_phase];
  return 1.0 / _fluid_viscosity[i][_phase];
}

Real
PorousFlowAdvectiveFluxCalculatorSaturated::computedU_dvar(unsigned i, unsigned pvar) const
{
  Real du = -_dfluid_viscosity_dvar[i][_phase][pvar] / std::pow(_fluid_viscosity[i][_phase], 2);
  if (_multiply_by_density)
    du = du * (*_fluid_density_node)[i][_phase] +
         (*_dfluid_density_node_dvar)[i][_phase][pvar] / _fluid_viscosity[i][_phase];
  return du;
}
